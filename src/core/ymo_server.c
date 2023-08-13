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

#define _GNU_SOURCE
#include "yimmo_config.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <stdatomic.h>


#include "yimmo.h"
#include "ymo_alloc.h"
#include "ymo_log.h"
#include "ymo_proto.h"
#include "ymo_util.h"
#include "ymo_server.h"
#include "ymo_conn.h"
#include "ymo_net.h"
#include "ymo_env.h"

#ifndef YMO_SERVER_TRACE
#  define YMO_SERVER_TRACE 1
#endif

#if defined(YMO_SERVER_TRACE) && YMO_SERVER_TRACE == 1
# define SERVER_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__);
# define SERVER_TRACE_UUID(fmt, ...) ymo_log_trace_uuid(fmt, __VA_ARGS__);
#else
# define SERVER_TRACE(fmt, ...)
# define SERVER_TRACE_UUID(fmt, ...)
#endif /* YMO_SERVER_TRACE */

#include "ymo_tls.h"

/*---------------------------------------------------------------*
 *  Utility Prototypes:
 *---------------------------------------------------------------*/
static int ymo_fd_accept(ymo_server_t* server, int revents);
static void ymo_conn_accept(ymo_server_t* server, int client_fd);
static ymo_status_t conn_proto_init(
        ymo_proto_t* proto,
        ymo_conn_t* conn);
static inline void close_and_free_connection(
        ymo_server_t* server, ymo_conn_t* conn, int clean);
static YMO_FUNC_UNUSED void sigpipe_noop_cb(int x);
static void idle_timeout_cb(bsat_toq_t* toq, bsat_timeout_t* item);


/*---------------------------------------------------------------*
 *  Yimmo Server API:
 *---------------------------------------------------------------*/
ymo_server_t* ymo_server_create(
        ymo_server_config_t* config,
        ymo_proto_t* proto)
{
    ymo_server_t* server = YMO_NEW0(ymo_server_t);
    if( !server ) {
        goto server_create_bail;
    }
    server->state = YMO_SERVER_CREATED;
    server->config = (*config);

    /* Set up protocol: */
    server->proto = proto;
    ymo_status_t proto_status =
        server->proto->vtable.init_cb(proto, server);
    if( proto_status != YMO_OKAY ) {
        /* Bail on server create, if proto init failed.
         * NOTE: this will invoke the proto cleanup function, if set.
         */
        goto server_create_free_and_bail;
    }

    server->cb_accept = ymo_accept_cb;
    return server;

server_create_free_and_bail:
    ymo_server_free(server);

server_create_bail:
    return NULL;
}


ymo_status_t ymo_server_init(ymo_server_t* server)
{
    ymo_status_t status = YMO_OKAY;

    ymo_log_info("%s:%i server starting...",
            server->proto->name, server->config.port);
    errno = 0;

    /* Create the socket. */
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if( listen_fd < 0 ) {
        ymo_log_fatal("Failed to create listen socket: %s", strerror(errno));
        return errno;
    }

    ymo_log_info("%s:%i listen FD: %i",
            server->proto->name, server->config.port, listen_fd);

    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_port = htons(server->config.port);
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;

    /* Configure SSL, if enabled: */
    if( server->config.cert_path && server->config.key_path ) {
        ymo_log_notice("TLS enabled: ✅");
        if( (status = ymo_init_ssl_ctx(server)) ) {
            ymo_log_fatal("SSL context initialization failed.");
            goto server_init_close_and_bail;
        }
        ymo_log_notice("TLS init okay: ✅");
    } else {
        ymo_log_notice("No TLS cert or path provided.");
        server->config.use_tls = 0;
    }

    /* Set socket traits: */
    if( (status = ymo_sock_reuse(listen_fd, server->config.flags)) ) {
        ymo_log_fatal("Failed to set server flags: %s", strerror(status));
        goto server_init_close_and_bail;
    }

    if( (status = ymo_sock_nonblocking(listen_fd)) ) {
        ymo_log_fatal("PID %i: Unable to put listen socket in non-blocking mode!",
                (int)getpid());
        goto server_init_close_and_bail;
    }

    /* Bind: */
    if( bind(listen_fd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) ) {
        ymo_log_fatal("Failed to bind to %i on %i: %s",
                server->config.port, listen_fd, strerror(errno));
        status = errno;
        goto server_init_close_and_bail;
    }
    ymo_log_info("%s:%i bind OK...",
            server->proto->name, server->config.port);

    /* Listen: */
    if( (status = ymo_server_init_socket(server, listen_fd)) ) {
        goto server_init_close_and_bail;
    }
    return status;

server_init_close_and_bail:
    close(listen_fd);
    return status;
}


ymo_status_t ymo_server_init_socket(ymo_server_t* server, int fd)
{
    if( listen(fd, server->config.listen_backlog) < 0 ) {
        ymo_log_fatal("Failed to listen to %i: %s", fd, strerror(errno));
        return errno;
    }
    ymo_log_info("%s:%i listen OK...",
            server->proto->name, server->config.port);

    /* Celebrate: */
    server->state = YMO_SERVER_INITIALIZED;
    server->listen_fd = fd;
    return YMO_OKAY;
}


/* If we're going to fork and share a common listen FD, we
 * should try to determine if there's contention BEFORE
 * invoking accept() and checking for EAGAIN.
 */
ymo_status_t ymo_server_pre_fork(ymo_server_t* server)
{
    if( server->accept_mutex ) {
        ymo_log_debug("%s",
                "ymo_server_pre_fork already invoked. "
                "(This isn't problematic. It's just not necessary)");
        return YMO_OKAY;
    }

    /* HACK HACK HACK: overprovision + never free, atm.. */
    void* shared = mmap(NULL, sizeof(pthread_mutex_t),
            PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED,
            -1, 0);
    if( shared == MAP_FAILED ) {
        ymo_log_error("Prefork failed: %s", strerror(errno));
        return ENOMEM;
    }

    /* Stick an mutex into the shared memory: */
    server->accept_mutex = shared;
    pthread_mutex_init(server->accept_mutex, NULL);

    /* Use the multiproc init, moving forward + restart. */
    server->cb_accept = ymo_multiproc_accept_cb;
    return YMO_OKAY;
}


ymo_status_t ymo_server_start(ymo_server_t* server, struct ev_loop* loop)
{
    ymo_log_info("%s:%i server starting...",
            server->proto->name, server->config.port);

    errno = 0;
    server->config.loop = loop;

    double def_timeout = YMO_SERVER_IDLE_TIMEOUT;
    double idle_timeout;
    if( ymo_env_as_double("YIMMO_SERVER_IDLE_TIMEOUT",
            &idle_timeout, &def_timeout) ) {
        int r_err = errno;
        ymo_log_error("Invalid YIMMO_SERVER_IDLE_TIMEOUT: %s",
                getenv("YIMMO_SERVER_IDLE_TIMEOUT"));
        return r_err;
    }

    ymo_log_info("%s:%i idle disconnect: %0.3fs",
            server->proto->name, server->config.port, idle_timeout);

    bsat_toq_init(
            loop,
            &(server->idle_toq),
            idle_timeout_cb,
            idle_timeout);
    server->idle_toq.data = server;

    ev_io_init(&server->w_accept, server->cb_accept, server->listen_fd, EV_READ);
    server->w_accept.data = server;

    ev_io_start(server->config.loop, &server->w_accept);

    ymo_log_info("%s:%i accept cb start OK...",
            server->proto->name, server->config.port);

    ymo_log_info("%s:%i started!",
            server->proto->name, server->config.port);
    server->state = YMO_SERVER_STARTED;
    return YMO_OKAY;
}


ymo_server_state_t ymo_server_get_state(ymo_server_t* server)
{
    return server->state;
}


struct ev_loop* ymo_server_loop(ymo_server_t* server)
{
    return server->config.loop;
}


ymo_status_t ymo_server_stop_graceful(ymo_server_t* server)
{
    int my_pid = (int)getpid();
    ymo_log_notice("%i: stopping accept watcher for socket on port %i",
            my_pid, server->config.port);
    ev_io_stop(server->config.loop, &server->w_accept);

    ymo_log_notice("%i: closing socket for port %i (fd: %i)",
            my_pid, server->config.port, server->listen_fd);
    close(server->listen_fd);
    server->state = YMO_SERVER_STOP_GRACEFUL;

    if( server->no_conn == 0 ) {
        ymo_log_notice("Graceful termination. Conn count == %zu. Breaking.",
                server->no_conn);
        ev_break(server->config.loop, EVBREAK_ALL);
    }
    /* TODO: set flag and/or start timer to actually ensure an exit */
    return YMO_OKAY;
}


void ymo_server_free(ymo_server_t* server)
{
    int my_pid = getpid();
    switch( server->state ) {
        case YMO_SERVER_STOP_GRACEFUL:
            YMO_STMT_ATTR_FALLTHROUGH();
        case YMO_SERVER_STARTED:
            ymo_log_notice("%i: Stopping accept watcher", my_pid);
            ev_io_stop(server->config.loop, &server->w_accept);
            YMO_STMT_ATTR_FALLTHROUGH();
        case YMO_SERVER_INITIALIZED:
            ymo_log_notice("%i: closing listen fd", my_pid);
            close(server->listen_fd);
            YMO_STMT_ATTR_FALLTHROUGH();
        case YMO_SERVER_CREATED:
            break;
    }

    bsat_toq_stop(&(server->idle_toq));

    if( server->proto->vtable.cleanup_cb ) {
        ymo_log_notice("%i: invoking proto cleanup callback", my_pid);
        server->proto->vtable.cleanup_cb(server->proto, server);
    }

#if YMO_ENABLE_TLS
    if( server->ssl_ctx ) {
        SSL_CTX_free(server->ssl_ctx);
    }
#endif /* YMO_ENABLE_TLS */
    ymo_log_notice("%i: freeing server object", my_pid);
    YMO_DELETE(ymo_server_t, server);
}


/* TODO: move to ymo_proto.c or ymo_conn.c: */
ymo_status_t ymo_conn_transition_proto(
        ymo_conn_t* conn, ymo_proto_t* proto_new)
{
    /* Cleanup the old protocol: */
    conn->proto->vtable.conn_cleanup_cb(
            conn->proto->data, conn, conn->proto_data);

    /* Transition to the new: */
    conn->proto = proto_new;
    return conn_proto_init(proto_new, conn);
}


/*=========================================================================*
 *
 * EV Callbacks:
 *
 *-------------------------------------------------------------------------*/

/*---------*
 * ACCEPT:
 *---------*/

void ymo_accept_cb(struct ev_loop* loop, struct ev_io* watcher, int revents)
{
    ymo_server_t* server = watcher->data;
    int client_fd = ymo_fd_accept(server, revents);

    if( client_fd > 0 ) {
        ymo_conn_accept(server, client_fd);
    } else {
        ymo_log_debug("Accept failed: %s", strerror(errno));
    }
    return;
}


void ymo_multiproc_accept_cb(
        struct ev_loop* loop, struct ev_io* watcher, int revents)
{
    ymo_server_t* server = watcher->data;

    if( pthread_mutex_trylock(server->accept_mutex) ) {
        /* TODO:
         * - use: EOWNERDEAD and pthread_mutex_consistent, where present
         * - elsewhere: use counter; if we NEVER accept, exit()
         */
        return;
    }

    int client_fd = ymo_fd_accept(server, revents);
    int accept_err = errno; /* Stash because pthreads could overwrite */
    pthread_mutex_unlock(server->accept_mutex);

    if( client_fd > 0 ) {
        ymo_conn_accept(server, client_fd);
    } else {
        ymo_log_debug("Accept failed: %s", strerror(accept_err));
    }

    return;
}


/*---------*
 * READ:
 *---------*/
void ymo_read_cb(struct ev_loop* loop, struct ev_io* watcher, int revents)
{
    ymo_conn_t* conn = (ymo_conn_t*)watcher->data;
    ymo_server_t* server = conn->server;


    if( EV_ERROR & revents ) {
        ymo_log_warning("libev error on read fd: %i", watcher->fd);
        if( conn ) {
            close_and_free_connection(server, conn, 0);
        }
        return;
    }

    /* Read the payload: */
    char* recv_buf;
    ssize_t len;
    ymo_status_t status;

do_read:
    errno = 0;
    status = YMO_OKAY;

    if( !CONN_SSL(conn) ) {
        len = recv(watcher->fd, server->recv_buf,
                YMO_SERVER_RECV_BUF_SIZE, YMO_RECV_FLAGS);
    } else {
        len = ymo_server_ssl_read(server, conn);
    }

    if( len < 0 ) {
        if( YMO_IS_BLOCKED(errno) ) {
            SERVER_TRACE("Read would block (conn: %p, fd: %i)",
                    (void*)conn, conn->id, NULL);
            return;
        } else {
            ymo_log_debug("Read error: %s (%i) on (conn: %p, fd: %i)",
                    strerror(errno), errno, (void*)conn, conn->fd);
            close_and_free_connection(server, conn, 0);
            return;
        }
    }

    /* Shut down the FD now and bail if the client closed the connection. */
    if( !len ) {
        SERVER_TRACE("Client terminated connection (conn: %p, fd: %i)",
                (void*)conn, conn->id, NULL);
        close_and_free_connection(server, conn, 0);
        return;
    }

    /* Invoke read callbacks if we're connected. Else, bounce back to "do_read"
     * until we either get a client close or an error:*/
    if( conn->state == YMO_CONN_OPEN
        || conn->state == YMO_CONN_TLS_ESTABLISHED ) {
        ymo_conn_reset_idle_timeout(conn, &(server->idle_toq));

        /* Dispatch to appropriate handler: */
        recv_buf = server->recv_buf;
        SERVER_TRACE("Read %li bytes from socket", len);
        do {
            ssize_t n = 0;

            SERVER_TRACE_UUID(
                    "Issuing %li bytes to parser", conn->id, len);

            n = conn->proto->vtable.read_cb(
                    conn->proto->data, conn, conn->proto_data, recv_buf, len);

            if( n >= 0 ) {
                recv_buf += n;
                len -= n;
            } else {
                status = errno;
                SERVER_TRACE("Breaking from read loop: %s", strerror(status));
                goto read_check_parse_error;
                break;
            }

            if( conn->state == YMO_CONN_SHUTDOWN ) {
                goto do_read;
            }
        } while( len );
    } else {
        SERVER_TRACE("Attempt additional read (conn: %p, fd: %i)",
                (void*)conn, conn->id, NULL);
        goto do_read;
    }

#if YMO_ENABLE_TLS && defined(YMO_CHECK_SSL_PENDING) && YMO_CHECK_SSL_PENDING
    if( (conn->state == YMO_CONN_TLS_ESTABLISHED
         || conn->state == YMO_CONN_TLS_CLOSING)
        && SSL_pending(conn->ssl) ) {
        goto do_read;
    }
#endif /* YMO_CHECK_SSL_PENDING */
    return;

read_check_parse_error:
    /* Bail on handler error: */
    if( status != YMO_OKAY && !YMO_IS_BLOCKED(status) ) {
        ymo_log_debug("Parse error (%s), closing connection (conn: %p, fd: %i)",
                strerror(status), (void*)conn, conn->fd);
        close_and_free_connection(server, conn, 0);
        return;
    }

    return;
}


/*---------*
 * WRITE:
 *------- -*/
void ymo_write_cb(struct ev_loop* loop, struct ev_io* watcher, int revents)
{
    ymo_conn_t* conn = (ymo_conn_t*)watcher->data;
    ymo_server_t* server = conn->server;

    ymo_status_t status = conn->proto->vtable.write_cb(
            conn->proto->data, conn, conn->proto_data, conn->fd);

    ymo_conn_reset_idle_timeout(conn, &(server->idle_toq));
    if( status == YMO_OKAY ) {
        /* No further data to send; leave the connection open: */
        ymo_conn_tx_enable(conn, 0);
    } else if( !YMO_IS_BLOCKED(status) ) {
        /* Error or conn close; close the socket: */
        close_and_free_connection(conn->server, conn, 1);
    }
    return;
}


/*===============================================================*
 *
 *  Utility:
 *
 *---------------------------------------------------------------*/
static int ymo_fd_accept(ymo_server_t* server, int revents)
{
#if YMO_ENABLE_TLS
    if( server->ssl_ctx ) {
        ERR_clear_error();
    }
#endif /* YMO_ENABLE_TLS */

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    if( EV_ERROR & revents ) {
        ymo_log_warning("libev error on accept fd: %i", server->listen_fd);
        return -1;
    }

    return accept(
            server->listen_fd, (struct sockaddr*)&client_addr, &client_len);
}


static void ymo_conn_accept(ymo_server_t* server, int client_fd)
{
    if( client_fd < 0 ) {
        if( YMO_IS_BLOCKED(errno) ) {
            ymo_log_debug("accept would block (%i); try again", client_fd);
        } else {
            ymo_log_warning("accept failed: %s (%i)",
                    strerror(errno), errno);
        }
        return;
    }

    ymo_client_sock_nonblocking(client_fd);
    ymo_client_sock_nosigpipe(client_fd);

    ymo_conn_t* conn = NULL;
    conn = ymo_conn_create(
            server, server->proto, client_fd, ymo_read_cb, ymo_write_cb);

    /* Bail on connection create failure: */
    if( !conn ) {
        SERVER_TRACE("Protocol connection create failed: %s (%i)",
                strerror(errno), errno);
        close(client_fd);
        return;
    }

    if( ymo_init_ssl(server, conn, client_fd) ) {
        SERVER_TRACE("SSL init failed: %s (%i)",
                strerror(errno), errno);
        close_and_free_connection(server, conn, 1);
        return;
    }

    /* TODO: split this. For TLS, we'll want to invoke ready callback
     * AFTER the handshake. */
    ymo_status_t init_status = conn_proto_init(server->proto, conn);
    if( init_status != YMO_OKAY ) {
        SERVER_TRACE("Connection initialization failed: %s (%i)",
                strerror(init_status), init_status);
        close_and_free_connection(server, conn, 1);
        return;
    }

    /* Issue user-level connection initialization callback.
     * TODO: Remove user-level CONNECTION data? (i.e. use proto instead?)
     */
    if( server->config.user_init ) {
        conn->user = server->config.user_init(server, conn);
    }

    server->no_conn++;
    SERVER_TRACE("Number of connections: %zu\n", server->no_conn);
    SERVER_TRACE_UUID("Session created; Enabling read for socket %i",
            conn->id, client_fd);
    /* Start the idle disconnect timer for this conn: */
    ymo_conn_start_idle_timeout(conn, &(server->idle_toq));
    ymo_conn_rx_enable(conn, 1);
}


static ymo_status_t conn_proto_init(
        ymo_proto_t* proto,
        ymo_conn_t* conn)
{
    conn->proto_data = proto->vtable.conn_init_cb(proto->data, conn);

    if( !conn->proto_data ) {
        SERVER_TRACE("Protocol data create failed: %s (%i)",
                strerror(errno), errno);
        return errno;
    }

    ymo_status_t init_status = YMO_OKAY;
    if( proto->vtable.conn_ready_cb ) {
        SERVER_TRACE_UUID("Invoking %s connect ready callback",
                conn->id, (const unsigned char*)proto->name);
        init_status = proto->vtable.conn_ready_cb(
                proto->data, conn, conn->proto_data);

        if( init_status != YMO_OKAY ) {
            SERVER_TRACE("Protocol ready invocation failed: %s (%i)",
                    strerror(errno), errno);
        }
    } else {
        SERVER_TRACE_UUID("No ready callback for %s",
                conn->id, (const unsigned char*)proto->name);
    }

    return init_status;
}


static inline void close_and_free_connection(
        ymo_server_t* server, ymo_conn_t* conn, int clean)
{
#if YMO_ENABLE_TLS
    /* If the connection had SSL, let's clean that up now: */
    if( conn->ssl ) {
        if( conn->state == YMO_CONN_TLS_ESTABLISHED ) {
            SERVER_TRACE("Cleaning up SSL for %i", conn->fd);
            SSL_shutdown(conn->ssl);
        }

        SSL_free(conn->ssl);
        conn->ssl = NULL;
    }

#endif /* YMO_ENABLE_TLS */

    /* If we're mid shutdown sequence, wait on another read sec: */
    if( ymo_conn_close(conn, clean) != YMO_CONN_CLOSED ) {
        return;
    }

    /* Invoke user cleanup, if present: */
    if( server->config.user_cleanup ) {
        SERVER_TRACE("Invoking user cleanup with %p", conn->user);
        server->config.user_cleanup(
                server, conn, conn->user);
    }

    /* Invoke proto cleanup, if present: */
    ymo_proto_t* proto = conn->proto;
    if( proto && proto->vtable.conn_cleanup_cb ) {
        SERVER_TRACE("Invoking proto cleanup with %p", conn->proto_data);
        proto->vtable.conn_cleanup_cb(
                proto->data, conn, conn->proto_data);
    }

    ymo_conn_free(conn);
    server->no_conn--;
    SERVER_TRACE("Number of connections: %zu\n", server->no_conn);

    /* If we're doing a graceful shutdown and that was the last connection,
     * let's bail out now.
     */
    if( server->state == YMO_SERVER_STOP_GRACEFUL && server->no_conn == 0 ) {
        ymo_log_notice("Graceful termination. Conn count == %zu. Breaking.",
                server->no_conn);
        ev_break(server->config.loop, EVBREAK_ALL);
    }
    return;
}


/* SIGPIPE handler if unable to block it: */
static YMO_FUNC_UNUSED void sigpipe_noop_cb(int x)
{
    return;
}


/* Read timeout callback, used to terminate conns after inactivity. */
static void idle_timeout_cb(bsat_toq_t* toq, bsat_timeout_t* item)
{
    ymo_server_t* server = toq->data;
    ymo_conn_t* conn = item->data;
    if( conn ) {
        SERVER_TRACE("Idle connection timer fired (conn: %p, fd: %i)",
                (void*)conn, conn->id, NULL);
        close_and_free_connection(server, conn, 1);
    }
    return;
}

