/*=============================================================================
 * libyimmo: Lightweight socket server framework
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



/** .. _connection: */

/** Connections
 * =============
 *
 */

#ifndef YMO_CONNECTION_H
#define YMO_CONNECTION_H

#include "yimmo_config.h"

#include <pthread.h>
#include <bsat.h>
#include <uuid/uuid.h>

#if YMO_ENABLE_TLS
#include <openssl/ssl.h>
#endif /* YMO_ENABLE_TLS */

#include "yimmo.h"


/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/**
 * .. :c:enum:: `ymo_conn_state_t`
 *
 *    :YMO_CONN_OPEN: Connection is established.
 *    :YMO_CONN_CLOSING: Shutdown has been invoked. Waiting for clean close.
 *    :YMO_CONN_CLOSED: File descriptor has been closed.
 *    :YMO_CONN_ERROR: Connection is in an error state and being automatically terminated.
 *
 */

YMO_ENUM8_TYPEDEF(ymo_conn_state) {
    YMO_CONN_OPEN,
    YMO_CONN_TLS_HANDSHAKE,
    YMO_CONN_TLS_ESTABLISHED,
    YMO_CONN_TLS_CLOSING,
    YMO_CONN_TLS_CLOSED,
    YMO_CONN_SHUTDOWN,
    YMO_CONN_CLOSING,
    YMO_CONN_CLOSED,
} YMO_ENUM8_AS(ymo_conn_state_t);

/** Internal structure used to manage a yimmo conn.
 */
struct ymo_conn {
    ymo_server_t*     server;          /* Pointer to managing server */
    ymo_proto_t*      proto;           /* Current protocol managing this connection */
    void*             proto_data;      /* Protocol-specific connection data */
    void*             user;            /* User-code per-connection data */
    bsat_toq_t*       toq;             /* HACK HACK: fix cancel for now. */
    bsat_timeout_t    idle_timeout;    /* Used to disconnect idle sessions */
    uuid_t            id;              /* Unique ID */
    int               fd;              /* The underlying file descriptor */
    struct ev_io      w_read;          /* Per-connection read watcher */
    struct ev_io      w_write;         /* Per-connection write watcher */
    ymo_conn_state_t  state;           /* Connection state */
#if YMO_ENABLE_TLS
    SSL*              ssl;             /* Optional SSL connection info */
#endif /* YMO_ENABLE_TLS */
#if defined(YMO_CONN_LOCK) && (YMO_CONN_LOCK == 1)
    pthread_mutexattr_t  lattr;        /* Per-connection mutex attributes */
    pthread_mutex_t      lock;         /* Per-connection mutex */
#endif /* YMO_CONN_LOCK */
};

/*---------------------------------------------------------------*
 * Functions
 *---------------------------------------------------------------*/

/** We typedef the callback signature for ev_io_watcher's, purely as
 * a matter of convenience.
 */
typedef void (*ymo_ev_io_cb_t)(
        struct ev_loop* loop,
        struct ev_io* watcher,
        int revents);


/** Create a new conn object.
 */
ymo_conn_t* ymo_conn_create(
        ymo_server_t* server, ymo_proto_t* proto, int client_fd,
        ymo_ev_io_cb_t read_cb, ymo_ev_io_cb_t write_cb);


/** Start idle disconnect timer for a given conn.
 */
void ymo_conn_start_idle_timeout(
        ymo_conn_t* conn, bsat_toq_t* idle_toq);


/** Reset idle disconnect timer for a given conn.
 */
void ymo_conn_reset_idle_timeout(
        ymo_conn_t* conn, bsat_toq_t* idle_toq);


/** Cancel idle disconnect timer for a given conn.
 */
void ymo_conn_cancel_idle_timeout(ymo_conn_t* conn);


/** Transition a connection to a new protocol.
 */
ymo_status_t ymo_conn_transition_proto(
        ymo_conn_t* conn, ymo_proto_t* proto_new);

/** Send buckets over the wire. */
ymo_status_t ymo_conn_send_buckets(
        ymo_conn_t* conn, ymo_bucket_t** head_p);


/** Turn receiving on/off, according to flag (0 = off; 1 = on)
 */
void ymo_conn_rx_enable(ymo_conn_t* conn, int flag);


/** Turn receiving on/off, according to flag (0 = off; 1 = on)
 */
void ymo_conn_tx_enable(ymo_conn_t* conn, int flag);


/** Trigger the write callback right now, as if ev_run had invoked it.
 */
void ymo_conn_tx_now(ymo_conn_t* conn);


/** Trigger the read callback right now, as if ev_run had invoked it.
 */
void ymo_conn_rx_now(ymo_conn_t* conn);


/** Close a conn object.
 *
 * :param conn: The connection to close.
 * :param clean: if ``1`` perform a clean close using ``shutdown``. If ``0``, just close the file descriptor.
 * :returns: the connection state after invocation.
 */
ymo_conn_state_t ymo_conn_close(ymo_conn_t* conn, int clean);


/** Free a conn object.
 */
void ymo_conn_free(ymo_conn_t* conn);

#endif /* YMO_CONNECTION_H */



