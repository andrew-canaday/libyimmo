/*=============================================================================
 *
 * Copyright (c) 2014 Andrew Canaday
 *
 * This file is part of libyimmo (sometimes referred to as "yimmo" or "ymo").
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




#ifndef YMO_HTTP_PROTO_HTTP_H
#define YMO_HTTP_PROTO_HTTP_H
#include "ymo_config.h"
#include "yimmo.h"
#include "ymo_http.h"
#include "core/ymo_net.h"
#include "core/ymo_bucket.h"

/** Protocol
 * ==========
 *
 */

#ifndef HTTP_MAX_BODY
#  define HTTP_MAX_BODY 4096
#endif


/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/**
 * .. note::
 *    These were in ymo_proto_http.c, but moved here
 *    to make testing a little easier...
 */

typedef struct ymo_http_upgrade_chain {
    ymo_http_upgrade_cb_t          cb;
    ymo_proto_t*                   proto;
    struct ymo_http_upgrade_chain* next;
} ymo_http_upgrade_chain_t;

typedef struct ymo_http_proto_data {
    ymo_http_session_init_cb_t     session_init;
    ymo_http_header_cb_t           header_cb;
    ymo_http_body_cb_t             body_cb;
    ymo_http_cb_t                  http_cb;
    ymo_http_session_cleanup_cb_t  session_cleanup;
    ymo_http_upgrade_chain_t*      upgrade_handler;
    void*                          data;
} ymo_http_proto_data_t;


/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

/** */
ymo_status_t ymo_proto_http_init(ymo_proto_t* proto, ymo_server_t* server);

/** */
void ymo_proto_http_cleanup(ymo_proto_t* proto, ymo_server_t* server);

/** */
void* ymo_proto_http_conn_init(void* proto_data, ymo_conn_t* conn);

/** */
void ymo_proto_http_conn_cleanup(
        void* proto_data, ymo_conn_t* conn,
        void* conn_data);


/** HTTP Parse main entry point.
 *
 * :param server: ymo_server issuing the parse exchange
 * :param session: HTTP session we're parsing
 * :param proto_data: HTTP-specific data created by proto init
 * :param recv_buf: server read buffer
 * :param len: number of bytes in recv_buf
 *
 * :returns: number of bytes parsed on success, -1 on failure.
 */
ssize_t ymo_proto_http_read(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        char* recv_buf,
        size_t len);

/** HTTP protocol write callback
 */
ymo_status_t ymo_proto_http_write(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        int socket);

#endif /* YMO_HTTP_PROTO_HTTP_H */



