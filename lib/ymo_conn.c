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


#include "ymo_config.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* TODO: Connection shouldn't need server internals.. */
#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_conn.h"
#include "ymo_server.h"
#include "ymo_alloc.h"

ymo_server_t* ymo_conn_server(const ymo_conn_t* conn)
{
    return conn->server;
}

ymo_proto_t* ymo_conn_proto(const ymo_conn_t* conn)
{
    return conn->proto;
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
        ymo_ev_cb_t read_cb, ymo_ev_cb_t write_cb)
{
    ymo_conn_t* conn = NULL;
    conn = YMO_NEW(ymo_conn_t);
    if( conn ) {
        conn->proto = proto;
        conn->fd = client_fd;
        conn->ev_flags = 0;
        ev_io_init(&conn->w_read, read_cb, client_fd, EV_READ);
        ev_io_init(&conn->w_write, write_cb, client_fd, EV_WRITE);
        conn->w_read.data = conn->w_write.data = (void*)conn;
        conn->server = server;
        conn->user = NULL;
        conn->toq = NULL;

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
    conn->toq = idle_toq;
    bsat_timeout_start(idle_toq, &(conn->idle_timeout));
}

void ymo_conn_reset_idle_timeout(
        ymo_conn_t* conn, bsat_toq_t* idle_toq)
{
    bsat_timeout_reset(idle_toq, &(conn->idle_timeout));
}

void ymo_conn_cancel_idle_timeout(ymo_conn_t* conn)
{
    if( conn->toq ) {
        bsat_timeout_stop(conn->toq, &(conn->idle_timeout));
    }
}

/* TODO: (server internals) This can be done more concisely: */
void ymo_conn_rx_enable(ymo_conn_t* conn, int flag)
{
    if( flag && !(conn->ev_flags & EV_READ) ) {
        conn->ev_flags |= EV_READ;
        ev_io_start(conn->server->config.loop, &conn->w_read);
    } else if( !flag && (conn->ev_flags & EV_READ) ) {
        conn->ev_flags &= (~EV_READ);
        ev_io_stop(conn->server->config.loop, &conn->w_read);
    }
}

/* TODO: (server internals) */
void ymo_conn_tx_enable(ymo_conn_t* conn, int flag)
{
    if( flag && !(conn->ev_flags & EV_WRITE) ) {
        conn->ev_flags |= EV_WRITE;
        ev_io_start(conn->server->config.loop, &conn->w_write);
    } else if( !flag && (conn->ev_flags & EV_WRITE) ) {
        conn->ev_flags &= (~EV_WRITE);
        ev_io_stop(conn->server->config.loop, &conn->w_write);
    }
}

void ymo_conn_tx_now(ymo_conn_t* conn)
{
    ev_invoke(conn->server->config.loop, &conn->w_write, EV_WRITE);
    return;
}

/* TODO: (server internals) Should we just pass idle_toq as a param?
 * Is user code ever going to call this directly?
 */
void ymo_conn_close(ymo_conn_t* conn)
{
    ymo_conn_cancel_idle_timeout(conn);
    ymo_conn_rx_enable(conn, 0);
    ymo_conn_tx_enable(conn, 0);
    close(conn->fd);
    return;
}

void ymo_conn_free(ymo_conn_t* conn)
{
    ymo_log_trace_uuid("Freeing conn %p", conn->id, (void*)conn);
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



