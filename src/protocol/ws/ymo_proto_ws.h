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




#ifndef YMO_WS_PROTO_WS_H
#define YMO_WS_PROTO_WS_H
#include "ymo_config.h"
#include "yimmo.h"
#include "ymo_ws.h"
#include "core/ymo_net.h"

/** Protocol
 * ================
 *
 */

/**---------------------------------------------------------------
 * Definitions
 *---------------------------------------------------------------*/


/**  */
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

/** Protocol version enum.
 */
YMO_ENUM8_TYPEDEF(ymo_ws_proto_version) {
    DRAFT_IETF_HYBI_THEWEBSOCKETPROTOCOL_00, /* INTERIM */
    DRAFT_IETF_HYBI_THEWEBSOCKETPROTOCOL_01, /* INTERIM */
    DRAFT_IETF_HYBI_THEWEBSOCKETPROTOCOL_02, /* INTERIM */
    DRAFT_IETF_HYBI_THEWEBSOCKETPROTOCOL_03, /* INTERIM */
    DRAFT_IETF_HYBI_THEWEBSOCKETPROTOCOL_04, /* INTERIM */
    DRAFT_IETF_HYBI_THEWEBSOCKETPROTOCOL_05, /* INTERIM */
    DRAFT_IETF_HYBI_THEWEBSOCKETPROTOCOL_06, /* INTERIM */
    DRAFT_IETF_HYBI_THEWEBSOCKETPROTOCOL_07, /* INTERIM */
    DRAFT_IETF_HYBI_THEWEBSOCKETPROTOCOL_08, /* INTERIM */
    /*----------------
     * 9-12: RESERVED
     *----------------*/
    RFC_6455 = 13, /* STANDARD */
} YMO_ENUM8_AS(ymo_ws_proto_version_t);

typedef struct ymo_ws_proto_data ymo_ws_proto_data_t;

/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

/** */
ymo_status_t ymo_proto_ws_init(
        ymo_proto_t* proto, ymo_server_t* server);

/** */
void ymo_proto_ws_cleanup(
        ymo_proto_t* proto, ymo_server_t* server);

/** */
void* ymo_proto_ws_conn_init(
        void* proto_data, ymo_conn_t* conn);

/** */
ymo_status_t ymo_proto_ws_conn_ready(
        void* proto_data, ymo_conn_t* conn, void* conn_data);

/** */
void ymo_proto_ws_conn_cleanup(
        void* proto_data, ymo_conn_t* conn, void* conn_data);

/** WS Parse main entry point.
 * :param server: ymo_server issuing the parse request
 * :param session: WS session we're parsing
 * :param proto_data: WS-specific data created by proto init
 * :param recv_buf: server read buffer
 * :param len: number of bytes in recv_buf
 *
 * Returns number of bytes parsed on success, -1 on failure.
 */
ssize_t ymo_proto_ws_read(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        char* recv_buf,
        size_t len);

/** WS protocol write callback.
 *
 */
ymo_status_t ymo_proto_ws_write(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        int socket);

/** WebSocket upgrade handler. Attempts upgrade, as per RFC6455.
 *
 */
ymo_http_upgrade_status_t ymo_ws_upgrade_cb(
        const char* hdr_upgrade,
        ymo_http_request_t* request,
        ymo_http_response_t* response);

#endif /* YMO_WS_PROTO_WS_H */

