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




/** The Server
 * ============
 *
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

/*---------------------------------------------------------------*
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

/*---------------------------------------------------------------*
 *  Functions
 *---------------------------------------------------------------*/

/** libev accept callback. */
void ymo_accept_cb(struct ev_loop* loop, struct ev_io* watcher, int revents);

/** libev recv callback. */
void ymo_read_cb(struct ev_loop* loop, struct ev_io* watcher, int revents);

/** libev recv callback. */
void ymo_write_cb(struct ev_loop* loop, struct ev_io* watcher, int revents);

#endif /* YMO_SERVER_H */



