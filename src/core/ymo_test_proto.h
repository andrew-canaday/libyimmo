/*=============================================================================
 * test/ymo_basic: Test basic functions of libyimmo.
 *
 * Copyright (c) 2014 Andrew Canaday
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *===========================================================================*/

#ifndef YMO_PROTO_TEST_H
#define YMO_PROTO_TEST_H

#include "yimmo_config.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ymo_tap.h"

#include "yimmo.h"
#include "ymo_attrs.h"
#include "ymo_alloc.h"
#include "ymo_server.h"
#include "ymo_proto.h"
#include "ymo_conn.h"
#include "ymo_net.h"

/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/** Server struct wrapper for unit testing. */
struct ymo_test_server {
    ymo_server_config_t  config;
    ymo_server_t*        server;
    ymo_proto_t*         proto;
    void*                proto_data;
};

/** Connection struct wrapper for unit testing. */
struct ymo_test_conn {
    struct ymo_test_server* server;
    ymo_conn_t*             conn;
    union {
        int  fd_pair[2];
        struct {
            int  fd_send;
            int  fd_read;
        };
    };
};

/** */
typedef struct ymo_test_server ymo_test_server_t;

/** */
typedef struct ymo_test_conn ymo_test_conn_t;


/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

void noop_bsat_handler(bsat_toq_t* toq, bsat_timeout_t* item)
{
    return;
}


/**
 * Utility to create a simple server for unit tests:
 *
 * - create a :c:struct:`ymo_server` which wraps ``proto``, but don't bind to the listen port.
 * - store the server config, server, protocol, and protocol-specific data in a :c:type:`ymo_test_server_t`.
 *
 * :param proto: the :c:type:`ymo_proto_t` to wrap.
 * :returns: a :c:type:`ymo_test_server_t` on success; ``NULL`` on failure.
 *
 */
static ymo_test_server_t* test_server_create(ymo_proto_t* proto)
{
    ymo_test_server_t* test_server = YMO_NEW(ymo_test_server_t);

    struct ev_loop* loop = ev_loop_new(0);
    memset(&test_server->config, 0, sizeof(ymo_server_config_t));
    test_server->config.loop = loop;
    test_server->config.port = 8080; /* TODO */

    test_server->server = ymo_server_create(&test_server->config, proto);
    if( !test_server->server ) {
        YMO_FREE(test_server);
        return NULL;
    }

    bsat_toq_init(
            loop,
            &(test_server->server->idle_toq),
            &noop_bsat_handler,
            5.0);
    test_server->server->idle_toq.data = test_server->server;
    test_server->proto = proto;
    test_server->proto_data = proto->data;
    return test_server;
}


/**
 * Utility to create a simple :c:type:`ymo_conn_t` for unit tests:
 *
 * :param server: the :c:type:`ymo_server_t` to associate this connection with.
 * :returns: a :c:type:`ymo_test_conn_t` on success; ``NULL`` on failure.
 *
 */
static ymo_test_conn_t* test_conn_create(ymo_test_server_t* test_server)
{
    ymo_test_conn_t* test_conn = YMO_NEW(ymo_test_conn_t);

    int rc = socketpair(PF_LOCAL, SOCK_STREAM, 0, test_conn->fd_pair);
    if( rc ) {
        YMO_FREE(test_conn);
        return NULL;
    }

    ymo_sock_nonblocking(test_conn->fd_send);
    ymo_sock_nonblocking(test_conn->fd_read);

    test_conn->conn = ymo_conn_create(
            test_server->server, test_server->proto,
            test_conn->fd_send, ymo_read_cb, ymo_write_cb);
    if( !test_conn->conn ) {
        YMO_FREE(test_conn);
        return NULL;
    }

    return test_conn;
}


static void test_conn_free(ymo_test_conn_t* test_conn)
{
    close(test_conn->fd_send);
    close(test_conn->fd_read);
    ymo_conn_free(test_conn->conn);
}


#endif /* YMO_PROTO_TEST_H */
