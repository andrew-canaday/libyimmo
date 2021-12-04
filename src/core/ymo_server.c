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
#include "ymo_config.h"

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

#define YMO_SERVER_TRACE 1
#if defined(YMO_SERVER_TRACE) && YMO_SERVER_TRACE == 1
# define SERVER_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__);
#else
# define SERVER_TRACE(fmt, ...)
#endif /* YMO_SERVER_TRACE */

/*---------------------------------------------------------------*
 *  Static Functions:
 *---------------------------------------------------------------*/
/* Convenience macro to close and free ymo conn: */
static inline void close_and_free_connection(
        ymo_server_t* server, ymo_conn_t* conn)
{
    ymo_proto_t* proto = conn->proto;
    ymo_conn_close(conn);  /* Close */

    /* Invoke user cleanup, if present: */
    if( server->config.user_cleanup ) {
        SERVER_TRACE("Invoking user cleanup with %p", conn->user);
        server->config.user_cleanup(
                server, conn, conn->user);
    }

    /* Invoke proto cleanup, if present: */
    if( proto && proto->vtable.conn_cleanup_cb ) {
        SERVER_TRACE("Invoking proto cleanup with %p", conn->proto_data);
        proto->vtable.conn_cleanup_cb(
                proto->data, conn, conn->proto_data);
    }

    ymo_conn_free(conn);  /* Free */
    server->no_conn--;
    SERVER_TRACE("%i server (%i) #connections: %zu",
            (int)getpid(), (int)server->config.port, server->no_conn);

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

static YMO_FUNC_UNUSED void ymo_sigpipe(int x)
{
    return;
}

/* Read timeout callback, used to terminate conns after inactivity. */
static void idle_timeout_cb(bsat_toq_t* toq, bsat_timeout_t* item)
{
    ymo_server_t* server = toq->data;
    ymo_conn_t* conn = item->data;
    if( conn ) {
        ymo_log_trace_uuid("Idle connection timer fired (fd: %i)",
                conn->id, NULL);
        close_and_free_connection(server, conn);
    }
    return;
}


/*---------------------------------------------------------------*
 *  Yimmo Server Functions:
 *---------------------------------------------------------------*/
ymo_server_t* ymo_server_create(
        ymo_server_config_t* config,
        ymo_proto_t* proto)
{
    ymo_server_t* server = YMO_NEW(ymo_server_t);
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

    /* HACK HACK HACK: overprovision + never free, atm.. */
    void* shared = mmap(NULL, sizeof(atomic_int)*16,
            PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED,
            -1, 0);
    if( shared == MAP_FAILED ) {
        goto server_create_free_and_bail;
    }

#if YIMMO_WSGI
    server->accepting = (atomic_int*)YMO_PTR_CEIL(shared);
    atomic_init(server->accepting, 0);
#endif /* YIMMO_WSGI */
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

    /* Set socket traits: */
    if( (status = ymo_sock_reuse(listen_fd, server->config.flags)) ) {
        ymo_log_fatal("Failed to set server flags: %s", strerror(status));
        goto server_init_close_and_bail;
    }
    ymo_log_info("%s:%i Server flags set...",
            server->proto->name, server->config.port);

    if( (status = ymo_sock_nonblocking(listen_fd)) ) {
        ymo_log_fatal("PID %i: Unable to put listen socket in non-blocking mode!",
                (int)getpid());
        goto server_init_close_and_bail;
    }
    ymo_log_info("%s:%i Listen socket in non-blocking mode...",
            server->proto->name, server->config.port);

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

    ev_io_init(&server->w_accept, ymo_accept_cb, server->listen_fd, EV_READ);
    server->w_accept.data = server;

    ev_io_start(server->config.loop,&server->w_accept);

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

static ymo_status_t ymo_server_conn_init(
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
        ymo_log_trace_uuid("Invoking %s connect ready callback",
                conn->id, (const unsigned char*)proto->name);
        init_status = proto->vtable.conn_ready_cb(
                proto->data, conn, conn->proto_data);

        if( init_status != YMO_OKAY ) {
            SERVER_TRACE("Protocol ready invocation failed: %s (%i)",
                    strerror(errno), errno);
            proto->vtable.conn_cleanup_cb(
                    proto->data, conn, conn->proto_data);
        }
    } else {
        ymo_log_trace_uuid("No ready callback for %s",
                conn->id, (const unsigned char*)proto->name);
    }

    return init_status;
}

void ymo_accept_cb(struct ev_loop* loop, struct ev_io* watcher, int revents)
{
    ymo_server_t* server = watcher->data;

    /* HACK HACK HACK: we shouldn't be adjusting server internals for
     * an external program! (yimmo-wsgi)
     */
#if YIMMO_WSGI
    int expected = 0;
    /* TODO: WARNING: if the process holding this mutex dies, we're blorked.
     * There should be some expiration set here...
     */
    if( !atomic_compare_exchange_strong(server->accepting, &expected, 1) ) {
        return;
    }
#endif /* YIMMO_WSGI */

    ymo_log_debug("accept callback invoked for %i", (int)getpid());
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    if( EV_ERROR & revents ) {
        ymo_log_warning("libev error on accept fd: %i", watcher->fd);
#if YIMMO_WSGI
        atomic_store(server->accepting, 0);
#endif /* YIMMO_WSGI */
        return;
    }

    ymo_log_debug("%i invoking accept()", (int)getpid());
    int client_fd = accept(
            watcher->fd, (struct sockaddr*)&client_addr, &client_len);
#if YIMMO_WSGI
    atomic_store(server->accepting, 0);
#endif /* YIMMO_WSGI */

    if( client_fd < 0 ) {
        if( YMO_IS_BLOCKED(errno) ) {
            ymo_log_debug("accept would block (%i); try again", client_fd);
        } else {
            ymo_log_warning("accept failed: %s (%i)",
                    strerror(errno), errno);
        }
        return;
    }

#ifdef DEBUG_SOCK_OPT
    ymo_client_sock_nonblocking(client_fd);
    ymo_client_sock_nosigpipe(client_fd);
    unsigned long linger;
    unsigned long send_buf;
    unsigned long recv_buf;
    unsigned int linger_len;
    unsigned int send_buf_len;
    unsigned int recv_buf_len;
    getsockopt(client_fd, SOL_SOCKET, SO_SNDBUF, (void*)&send_buf, &send_buf_len);
    getsockopt(client_fd, SOL_SOCKET, SO_RCVBUF, (void*)&recv_buf, &recv_buf_len);
    getsockopt(client_fd, SOL_SOCKET, SO_LINGER, (void*)&linger, &linger_len);
    printf("Socket options: send=%li; recv=%li; linger=%li\n", send_buf, recv_buf, linger);
#endif /* DEBUG_SOCK_OPT */

    ymo_log_debug("%i: Accepted new connection on %i", (int)getpid(), client_fd);
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

    ymo_status_t init_status = ymo_server_conn_init(server->proto, conn);
    if( init_status != YMO_OKAY ) {
        SERVER_TRACE("Connection initialization failed: %s (%i)",
                strerror(init_status), init_status);
        close_and_free_connection(server, conn);
        return;
    }

    /* Issue user-level connection initialization callback.
     * TODO: Remove user-level CONNECTION data? (i.e. use proto instead?)
     * TODO: Otherwise...double check your API. Can they swap it?
     */
    if( server->config.user_init ) {
        conn->user = server->config.user_init(server, conn);
    }

    server->no_conn++;
    SERVER_TRACE("%i server (%i) #connections: %zu",
            (int)getpid(), (int)server->config.port, server->no_conn);
    ymo_log_trace_uuid("Session created; Enabling read for socket %i",
            conn->id, client_fd);
    /* Start the idle disconnect timer for this conn: */
    ymo_conn_start_idle_timeout(conn, &(server->idle_toq));
    ymo_conn_rx_enable(conn, 1);
    return;
}

void ymo_read_cb(struct ev_loop* loop, struct ev_io* watcher, int revents)
{
    if( EV_ERROR & revents ) {
        ymo_log_warning("libev error on read fd: %i", watcher->fd);
        goto read_cb_okay;
    }

    ymo_conn_t* conn = (ymo_conn_t*)watcher->data;

    /* Reset the idle disconnect timer: */
    ymo_server_t* server = conn->server;
    ymo_conn_reset_idle_timeout(conn, &(server->idle_toq));

    /* Read the payload: */
    char* recv_buf;
    ssize_t len = recv(watcher->fd, server->recv_buf,
            YMO_SERVER_RECV_BUF_SIZE, YMO_RECV_FLAGS);
    if( len < 0 ) {
        if( YMO_IS_BLOCKED(errno) ) {
            ymo_log_debug("Read would block (%i); returning", watcher->fd);
            goto read_cb_okay;
        } else {
            ymo_log_debug("Read error: %s (%i) on fd %i",
                    strerror(errno), errno, watcher->fd);
            goto read_cb_close_and_free;
        }
    }

    /* Bail on read error: */
    if( !len ) {
        ymo_log_debug("Client connection closed (fd: %i)", watcher->fd);
        goto read_cb_close_and_free;
    }

    /* Dispatch to appropriate handler: */
    ymo_status_t status = YMO_OKAY;
    recv_buf = server->recv_buf;
    SERVER_TRACE("Read %li bytes from socket", len);
    // SERVER_TRACE("Read \"%.*s\" from socket", len, recv_buf);
    do {
        ssize_t n = 0;

        ymo_log_trace_uuid(
                "Issuing %li bytes to parser", conn->id, len);

        n = conn->proto->vtable.read_cb(
                conn->proto->data, conn, conn->proto_data, recv_buf, len);

        if( n >= 0 ) {
#ifdef YMO_SERVER_PARANOIA
            /* If a protocol callback reads past the end of the buffer;
             * stomp hard on the connection.
             */
            if( n > len ) {
                ymo_log_error("Protocol \"%s\" read past end of recv buffer!",
                        conn->proto->name);
                status = EOVERFLOW;
                goto read_cb_close_and_free;
            }
#endif /* YMO_SERVER_PARANOIA */
            recv_buf += n;
            len -= n;
        } else {
            status = errno;
            SERVER_TRACE(
                    "Breaking from read loop: %s", strerror(errno));
            break;
        }
    } while( len );

    /* Bail on handler error: */
    if( status != YMO_OKAY && !YMO_IS_BLOCKED(status) ) {
        ymo_log_debug("Parse error (%s), closing connection (fd: %i)",
                strerror(status), watcher->fd);
        goto read_cb_close_and_free;
    }

read_cb_okay:
    return;

read_cb_close_and_free:
    close_and_free_connection(server, conn);
    return;
}

void ymo_write_cb(struct ev_loop* loop, struct ev_io* watcher, int revents)
{
    ymo_conn_t* conn = (ymo_conn_t*)watcher->data;
    ymo_server_t* server = conn->server;
    ymo_conn_reset_idle_timeout(conn, &(server->idle_toq));

    ymo_status_t status = conn->proto->vtable.write_cb(
            conn->proto->data, conn, conn->proto_data, conn->fd);

    if( status == YMO_OKAY ) {
        /* No further data to send; leave the connection open: */
        ymo_conn_tx_enable(conn, 0);
    } else if( !YMO_IS_BLOCKED(status) ) {
        /* Error or conn close; close the socket: */
        close_and_free_connection(conn->server, conn);
    }
    return;
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
    ymo_log_notice("%i: freeing server object", my_pid);
    YMO_DELETE(ymo_server_t, server);
}


/*---------------------------------------------------------------*
 *  Yimmo Connection Functions:
 *---------------------------------------------------------------*/

/* TODO: Blehck! This shouldn't be in here... */
ymo_status_t ymo_conn_transition_proto(
        ymo_conn_t* conn, ymo_proto_t* proto_new)
{
    /* Cleanup the old protocol: */
    conn->proto->vtable.conn_cleanup_cb(
            conn->proto->data, conn, conn->proto_data);

    /* Transition to the new: */
    conn->proto = proto_new;
    return ymo_server_conn_init(proto_new, conn);
}



