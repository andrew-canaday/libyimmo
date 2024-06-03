/*=============================================================================
 *
 *  Copyright (c) 2014 Andrew Canaday
 *
 *  This file is part of libyimmo (sometimes referred to as "yimmo" or "ymo").
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *===========================================================================*/


#include "yimmo_config.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if YMO_ENABLE_TLS
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif /* YMO_ENABLE_TLS */

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_conn.h"
#include "ymo_bucket.h"
#include "ymo_net.h"
#include "ymo_server.h"
#include "ymo_alloc.h"

#define YMO_CONN_TRACE 0
#if defined(YMO_CONN_TRACE) && YMO_CONN_TRACE == 1
# define CONN_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__);
# define CONN_TRACE_UUID(fmt, ...) ymo_log_trace_uuid(fmt, __VA_ARGS__);
#else
# define CONN_TRACE(fmt, ...)
# define CONN_TRACE_UUID(fmt, ...)
#endif /* YMO_CONN_TRACE */

static const char* c_state_names[] = {
    "YMO_CONN_OPEN",
    "YMO_CONN_TLS_HANDSHAKE",
    "YMO_CONN_TLS_ESTABLISHED",
    "YMO_CONN_TLS_CLOSING",
    "YMO_CONN_TLS_CLOSED",
    "YMO_CONN_SHUTDOWN",
    "YMO_CONN_CLOSING",
    "YMO_CONN_CLOSED",
};

ymo_server_t* ymo_conn_server(const ymo_conn_t* conn)
{
    return conn->server;
}


ymo_proto_t* ymo_conn_proto(const ymo_conn_t* conn)
{
    return conn->proto;
}


struct ev_loop* ymo_conn_loop(const ymo_conn_t* conn)
{
    return conn->loop;
}


void ymo_conn_id(uuid_t dst, const ymo_conn_t* conn)
{
    uuid_copy(dst, conn->id);
    return;
}


char* ymo_conn_id_str(const ymo_conn_t* conn)
{
    char id_str[37];
    uuid_unparse(conn->id, id_str);
    return strndup(id_str, 37);
}


ymo_conn_t* ymo_conn_create(
        ymo_server_t* server, ymo_proto_t* proto, int client_fd,
        struct ev_loop* loop, ymo_ev_io_cb_t read_cb, ymo_ev_io_cb_t write_cb)
{
    ymo_conn_t* conn = NULL;
    conn = YMO_NEW(ymo_conn_t);
    if( conn ) {
        conn->proto = proto;
        conn->fd = client_fd;
        conn->loop = loop;
        ev_io_init(&conn->w_read, read_cb, client_fd, EV_READ);
        ev_io_init(&conn->w_write, write_cb, client_fd, EV_WRITE);
        conn->w_read.data = conn->w_write.data = (void*)conn;
        conn->server = server;
        conn->user = NULL;
        conn->toq = NULL;
        conn->state = YMO_CONN_OPEN;

        bsat_timeout_init(&(conn->idle_timeout));
        conn->idle_timeout.data = conn;
#if defined(YMO_CONN_LOCK) && (YMO_CONN_LOCK == 1)
        pthread_mutexattr_settype(
                &conn->lattr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(
                &conn->lock, &conn->lattr);
#endif /* YMO_CONN_LOCK */
        uuid_generate(conn->id);
    }
    return conn;
}


void ymo_conn_start_idle_timeout(
        ymo_conn_t* conn, bsat_toq_t* idle_toq)
{
    CONN_TRACE("State at invocation: %s (conn: %p, fd: %i)",
            c_state_names[conn->state], (void*)conn, conn->fd);
    conn->toq = idle_toq;
    bsat_timeout_start(idle_toq, &(conn->idle_timeout));
}


void ymo_conn_reset_idle_timeout(
        ymo_conn_t* conn, bsat_toq_t* idle_toq)
{
    CONN_TRACE("State at invocation: %s (conn: %p, fd: %i)",
            c_state_names[conn->state], (void*)conn, conn->fd);
    switch( conn->state ) {
        /* Okay to restart: */
        case YMO_CONN_OPEN:
        /* fallthrough */
        case YMO_CONN_TLS_HANDSHAKE:
        /* fallthrough */
        case YMO_CONN_TLS_ESTABLISHED:
            /* fallthrough */
            bsat_timeout_reset(idle_toq, &(conn->idle_timeout));
            break;

        /* Okay to make sure it's armed, if it hasn't already timed out: */
        case YMO_CONN_TLS_CLOSING:
            break;
        case YMO_CONN_TLS_CLOSED:
            break;
        case YMO_CONN_SHUTDOWN:
        /* fallthrough */
        case YMO_CONN_CLOSING:
            bsat_timeout_start(idle_toq, &(conn->idle_timeout));
            break;

        /* Disallowed: */
        case YMO_CONN_CLOSED:
            ymo_log_debug("Attempt to re-arm idle watcher when closed (conn: %p, fd: %i)",
                    (void*)conn, conn->fd);
            break;
    }
}


void ymo_conn_cancel_idle_timeout(ymo_conn_t* conn)
{
    if( conn->toq ) {
        bsat_timeout_stop(conn->toq, &(conn->idle_timeout));
    }
}


typedef void (*ymo_ev_io_toggle)(EV_P_ struct ev_io* w);
static ymo_ev_io_toggle io_toggle[2] = {
    &ev_io_stop,
    &ev_io_start,
};


void ymo_conn_rx_enable(ymo_conn_t* conn, int flag)
{
    CONN_TRACE("RX-->%i; State at invocation: %s (conn: %p, fd: %i)",
            flag, c_state_names[conn->state], (void*)conn, conn->fd);

    io_toggle[flag & 0x01](conn->loop, &conn->w_read);
}


void ymo_conn_tx_enable(ymo_conn_t* conn, int flag)
{
    CONN_TRACE("TX-->%i; State at invocation: %s (conn: %p, fd: %i)",
            flag, c_state_names[conn->state], (void*)conn, conn->fd);
    io_toggle[flag & 0x01](conn->loop, &conn->w_write);
}


/** Send buckets over the wire. */
ymo_status_t ymo_conn_send_buckets(
        ymo_conn_t* conn, ymo_bucket_t** head_p)
{
#if !(YMO_ENABLE_TLS)
    return ymo_net_send_buckets(conn->fd, head_p);
#else
    /* HACK HACK HACK HACK
     * TODO: add ymo_net_send_tls or similar to ymo_net.
     */
    ymo_status_t status = YMO_OKAY;

    if( !conn->ssl ) {
        return ymo_net_send_buckets(conn->fd, head_p);
    }

    ymo_bucket_t* cur = *head_p;
    size_t bytes_sent = 0;

    do {
        int send_rc = SSL_write_ex(conn->ssl,
                (const char*)(cur->data + cur->bytes_sent),
                cur->len - cur->bytes_sent,
                &bytes_sent);

        if( send_rc > 0 ) {
            cur->bytes_sent += bytes_sent;

            if( cur->bytes_sent < cur->len ) {
                status = EAGAIN;
                break;
            }
            ymo_bucket_t* done = cur;
            cur = cur->next;
            ymo_bucket_free(done);
        } else {
            int ssl_err = SSL_get_error(conn->ssl, send_rc);
            if( YMO_SSL_WANT_RW(ssl_err) ) {
                ymo_log_debug("SSL buffering on write to %i", conn->fd);
                status = EAGAIN;
                break;
            } else {
                ymo_log_debug(
                        "SSL write for connection on fd %i "
                        "failed error code %i: (%s)",
                        conn->fd, ssl_err, ERR_reason_error_string(ssl_err));
                status = ECONNABORTED;
                break;
            }
        }
    } while( cur );

    *head_p = cur;
    return status;
#endif /* YMO_ENABLE_TLS */
}


void ymo_conn_tx_now(ymo_conn_t* conn)
{
    ev_invoke(conn->loop, &conn->w_write, EV_WRITE);
    return;
}


void ymo_conn_rx_now(ymo_conn_t* conn)
{
    ev_invoke(conn->loop, &conn->w_read, EV_READ);
    return;
}


ymo_status_t ymo_conn_shutdown(ymo_conn_t* conn)
{
    CONN_TRACE("State at invocation: %s (conn: %p, fd: %i)",
            c_state_names[conn->state], (void*)conn, conn->fd);

    ymo_status_t rc = YMO_OKAY;
    switch( conn->state ) {
        case YMO_CONN_TLS_ESTABLISHED:
        /* fallthrough */
        case YMO_CONN_TLS_CLOSING:
        /* fallthrough */
        case YMO_CONN_TLS_HANDSHAKE:
        /* fallthrough */
        case YMO_CONN_TLS_CLOSED:
        /* fallthrough */
        case YMO_CONN_OPEN:
            if( shutdown(conn->fd, SHUT_WR) ) {
                rc = errno;
                ymo_log_debug("Failed to shut down socket %i: %s",
                        conn->fd, strerror(rc));
                break;
            }

            conn->state = YMO_CONN_SHUTDOWN;
        /* fallthrough */
        case YMO_CONN_SHUTDOWN:
        /* fallthrough */
        case YMO_CONN_CLOSING:
            ymo_conn_rx_enable(conn, 1);
            ymo_conn_tx_enable(conn, 0);
        default:
            break;
    }

    CONN_TRACE("State at return: %s (conn: %p, fd: %i)",
            c_state_names[conn->state], (void*)conn, conn->fd);
    return rc;
}


ymo_conn_state_t ymo_conn_close(ymo_conn_t* conn, int clean)
{
    CONN_TRACE("Clean: %i; State at invocation: %s (conn: %p, fd: %i)",
            clean, c_state_names[conn->state], (void*)conn, conn->fd);
    static char junk_buffer[64];

    /* If clean is false, jump right to closing the fd without doing the
     * ol' shutdown + recv rigmarole.
     */
    if( !clean ) {
        conn->state = YMO_CONN_CLOSING;
    }

    switch( conn->state ) {
        case YMO_CONN_TLS_ESTABLISHED:
        /* fallthrough */
        case YMO_CONN_TLS_CLOSING:
        /* fallthrough */
        case YMO_CONN_TLS_HANDSHAKE:
        /* fallthrough */
        case YMO_CONN_TLS_CLOSED:
        /* fallthrough */
        case YMO_CONN_OPEN:
            ymo_conn_shutdown(conn);
        /* fallthrough */
        case YMO_CONN_SHUTDOWN:
        {
            int rc = 0;
            size_t len = 0;
            do {
                errno = 0;
                len = recv(conn->fd, junk_buffer, 64, YMO_RECV_FLAGS);
            } while( len > 0 );

            rc = errno;

            if( clean && len < 0 && YMO_IS_BLOCKED(rc) ) {
                break;
            }
            conn->state = YMO_CONN_CLOSING;
        }

        case YMO_CONN_CLOSING:
            CONN_TRACE("Performing close (conn: %p, fd: %i)",
                    (void*)conn, conn->fd);
            ymo_conn_cancel_idle_timeout(conn);
            ymo_conn_tx_enable(conn, 0);
            shutdown(conn->fd, SHUT_RDWR);
            close(conn->fd);
            conn->state = YMO_CONN_CLOSED;
        /* fallthrough */
        case YMO_CONN_CLOSED:
            ymo_conn_rx_enable(conn, 0);
            ymo_conn_tx_enable(conn, 0);
        default:
            /* fallthrough */
            break;
    }

    CONN_TRACE("State at return: %s (conn: %p, fd: %i)",
            c_state_names[conn->state], (void*)conn, conn->fd);
    return conn->state;
}


void ymo_conn_free(ymo_conn_t* conn)
{
    CONN_TRACE_UUID("Freeing conn %p", conn->id, (void*)conn);
    YMO_DELETE(ymo_conn_t, conn);
    return;
}


#if defined(YMO_CONN_LOCK) && (YMO_CONN_LOCK == 1)
void ymo_conn_lock(ymo_conn_t* conn)
{
    pthread_mutex_lock(&conn->lock);
}


void ymo_conn_unlock(ymo_conn_t* conn)
{
    pthread_mutex_unlock(&conn->lock);
}


#endif /* YMO_CONN_LOCK */



