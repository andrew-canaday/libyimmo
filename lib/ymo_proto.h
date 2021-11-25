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




/** Protocols
 * ===========
 *
 */

#ifndef YMO_PROTOCOL_H
#define YMO_PROTOCOL_H
#include "ymo_config.h"
#include <stddef.h>
#include "yimmo.h"

/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/


/** Protocol alloc/init callback type. [REQUIRED]
 *
 * Individual protocols provide functions which match this signature in order
 * to provide allocation/initialization.
 *
 * :proto_data: private data for this protocol
 * :server: the server issuing the callback
 *
 * Returns a status indicating protocol is ready for use
 *
 * be separated into two distinct callbacks.
 */
typedef ymo_status_t (* ymo_proto_init_cb_t)(
        ymo_proto_t* proto, ymo_server_t* server);

/** Protocol-level cleanup/dealloc callback type. [REQUIRED]
 *
 * Individual protocols provide a function which matches this signature in
 * order to perform cleanup/deallocation.
 *
 * :proto: the protocol being destroyed
 * :server: the server which is issuing the callback
 *
 * be separated into two distinct callbacks.
 */
typedef void (* ymo_proto_cleanup_cb_t)(
        ymo_proto_t* proto, ymo_server_t* server);

/** Protocol-level connection init callback. [OPTIONAL]
 *
 * Gives protocols a chance to do alloc/init per-connection data.
 *
 * :proto_data: the protocol data against which this is being invoked.
 * :conn: the newly created connection object
 *
 * Returns per-connection protocol data (or NULL)
 */
typedef void* (* ymo_proto_conn_init_cb_t)(
        void* proto_data, ymo_conn_t* conn);

/** Protocol-level connection ready callback. [OPTIONAL]
 *
 * Gives protocols a chance to do "on-open" style notification to user space.
 *
 * :proto_data: the protocol data against which this is being invoked.
 * :conn: the newly created connection object
 * :returns: per-connection protocol data (or NULL)
 */
typedef ymo_status_t (* ymo_proto_conn_ready_cb_t)(
        void* proto_data, ymo_conn_t* conn, void* conn_data);

/** Protocol-level connection cleanup callback.
 *
 * Gives protocols a chance to do cleanup/deallocate per-connection data.
 *
 * :proto_data: the protocol data against which this is being invoked
 * :conn: the connection object that's being destroyed
 * :conn_data: the per-connection data associated with this connection
 */
typedef void (* ymo_proto_conn_cleanup_cb_t)(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data);

/** Protocol-level read callback. */
typedef ssize_t (* ymo_proto_read_cb_t)(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        char* buf_in,
        size_t len);

/** Protocol-level write callback. */
typedef ymo_status_t (* ymo_proto_write_cb_t)(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        int socket);

/* Predeclarations Definition below. */
typedef struct ymo_proto_vt ymo_proto_vt_t;

/** Data structure used to provide callback mappings for a protocol. */
struct ymo_proto_vt {
    ymo_proto_init_cb_t          init_cb;         /* Protocol init callback */
    ymo_proto_cleanup_cb_t       cleanup_cb;      /* Protocol cleanup callback */
    ymo_proto_conn_init_cb_t     conn_init_cb;    /* Client init callback */
    ymo_proto_conn_ready_cb_t    conn_ready_cb;   /* Client ready callback */
    ymo_proto_conn_cleanup_cb_t  conn_cleanup_cb; /* Client cleanup callback */
    ymo_proto_read_cb_t          read_cb;         /* Protocol read callback */
    ymo_proto_write_cb_t         write_cb;        /* Protocol write callback */
};

/** Data structure used to define a single protocol. */
struct ymo_proto {
    const char*          name; /* Plain text protocol name */
    void*                data;
    struct ymo_proto_vt  vtable;
};

#endif /* YMO_PROTOCOL_H */

