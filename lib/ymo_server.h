/*=============================================================================
 * libyimmo: Lightweight socket server framework
 *
 *  Copyright (c) 2014 Andrew Canaday
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


/** .. _server: */

/** Server
 * ========
 *
 * The :c:struct:`ymo_server` is a relatively plain, TCP socket server. It
 * is responsible for binding and listening on the server socket, handling
 * connections (accepting, closing, and idle disconnects), and is responsible
 * for sending and receiving raw data over the wire.
 *
 * Server objects are paired with :ref:`protocols`, which handle higher-level
 * functions like parsing and domain-specific user-facing functionality. Data
 * and event notifications are relayed to protocols via a set of
 * :ref:`protocol callbacks`.
 *
 * Data is read from the wire into a *single* server-owned read buffer.
 * From there, it is dispatched to the read handler for a specific
 * :c:type:`ymo_proto_t`. The protocol is *free to modify the contents of the
 * receive buffer*, but be aware: *the buffer is considered free for reuse
 * by the server after the protocol read callback has returned!*
 *
 * .. admonition:: Info
 *
 *    For more information on parsing, see :ref:`protocols`.
 *
 */

#ifndef YMO_SERVER_H
#define YMO_SERVER_H
#include "ymo_config.h"

#if defined(YIMMO_WSGI) && (YIMMO_WSGI == 1)
#include <stdatomic.h>
#endif /* YIMMO_WSGI */

#include <ev.h>
#include <bsat.h>

#include "yimmo.h"
#include "ymo_proto.h"
#include "ymo_trie.h"

#include "ymo_proto.h"

/**---------------------------------------------------------------
 *  Types
 *---------------------------------------------------------------*/

/** Internal structure used to manage a yimmo server. */
struct ymo_server {
    struct ev_io         w_accept;                           /* EV IO watcher for events on accept socket */
    char                 recv_buf[YMO_SERVER_RECV_BUF_SIZE]; /* TODO: configure @ runtime */
    bsat_toq_t           idle_toq;                           /* Used for idle disconnect timeouts */
    ymo_proto_t*         proto;                              /* Primary protocol for this server */
    ymo_server_config_t  config;
    int                  listen_fd; /* Socket for `listen`/`accept` */
    ymo_server_state_t   state;
    size_t               no_conn;
#if defined(YIMMO_WSGI) && (YIMMO_WSGI == 1)
    atomic_int*          accepting;
#endif /* YIMMO_WSGI */
};

/**---------------------------------------------------------------
 *  Functions
 *---------------------------------------------------------------*/

/**
 * This is the libev accept callback. It is invoked whenever a readiness
 * notification is received from libev on the server *listen* socket.
 */
void ymo_accept_cb(struct ev_loop* loop, struct ev_io* watcher, int revents);

/**
 * This is the libev read callback. It is invoked whenever a readiness
 * notification is received from libev on a particular *client connection*
 * socket indicating that a read may be performed.
 */
void ymo_read_cb(struct ev_loop* loop, struct ev_io* watcher, int revents);

/**
 * This is the libev write callback. It is invoked whenever a readiness
 * notification is received from libev on a particular *client connection*
 * socket, indicating that a write may be performed.
 */
void ymo_write_cb(struct ev_loop* loop, struct ev_io* watcher, int revents);

#endif /* YMO_SERVER_H */



