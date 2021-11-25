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




/** Connections
 * =============
 *
 */

#ifndef YMO_CONNECTION_H
#define YMO_CONNECTION_H
#include "ymo_config.h"

#include <pthread.h>
#include <bsat.h>
#include <uuid/uuid.h>

#include "yimmo.h"


/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

YMO_ENUM8_TYPEDEF(conn_state) {
    CONNECTION_STATE_NO_CONNECTION,
    CONNECTION_STATE_OPEN,
    CONNECTION_STATE_CLOSED,
    CONNECTION_STATE_CLOSING,
} YMO_ENUM8_AS(conn_state_t);

/** Internal structure used to manage a yimmo conn.
 */
struct ymo_conn {
    ymo_server_t*        server;       /* Pointer to managing server */
    ymo_proto_t*         proto;        /* Current protocol managing this connection */
    void*                proto_data;   /* Protocol-specific connection data */
    void*                user;         /* User-code per-connection data */
    bsat_toq_t*          toq;          /* HACK HACK: fix cancel for now. */
    bsat_timeout_t       idle_timeout; /* Used to disconnect idle sessions */
    uuid_t               id;           /* Unique ID */
    int                  fd;           /* The underlying file descriptor */
    int                  ev_flags;     /* Used to store EV_READ/EV_WRITE flags */
    struct ev_io         w_read;       /* Per-connection read watcher */
    struct ev_io         w_write;      /* Per-connection write watcher */
    pthread_mutexattr_t  lattr;        /* Per-connection mutex attributes */
    pthread_mutex_t      lock;         /* Per-connection mutex */
};

/*---------------------------------------------------------------*
 * Functions
 *---------------------------------------------------------------*/


/** Create a new conn object.
 */
ymo_conn_t* ymo_conn_create(
        ymo_server_t* server, ymo_proto_t* proto, int client_fd,
        ymo_ev_cb_t read_cb, ymo_ev_cb_t write_cb);


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


/** Turn receiving on/off, according to flag (0 = off; 1 = on)
 */
void ymo_conn_rx_enable(ymo_conn_t* conn, int flag);


/** Turn receiving on/off, according to flag (0 = off; 1 = on)
 */
void ymo_conn_tx_enable(ymo_conn_t* conn, int flag);


/** Trigger the write callback right now, as if ev_run had invoked it.
 */
void ymo_conn_tx_now(ymo_conn_t* conn);


/** Close a conn object.
 */
void ymo_conn_close(ymo_conn_t* conn);


/** Free a conn object.
 */
void ymo_conn_free(ymo_conn_t* conn);

#endif /* YMO_CONNECTION_H */



