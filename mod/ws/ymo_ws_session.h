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




#ifndef YMO_WS_SESSION_H
#define YMO_WS_SESSION_H
#include "ymo_config.h"
#include <stdio.h>
#include <arpa/inet.h>

#include "yimmo.h"
#include "ymo_ws.h"
#include "ymo_proto_ws.h"

/** Session
 * =========
 *
 */

/**---------------------------------------------------------------
 * Types:
 *---------------------------------------------------------------*/

YMO_ENUM8_TYPEDEF(ws_parse_state) {
    WS_PARSE_OP,
    WS_PARSE_LEN,
    WS_PARSE_LEN_EXTENDED,
    WS_PARSE_MASKING_KEY,
    WS_PARSE_PAYLOAD,
    WS_PARSE_COMPLETE,
} YMO_ENUM8_AS(ws_parse_state_t);


/** Defines WS message frame flags. */
typedef union ymo_ws_frame_flags {
    uint8_t  packed;
    struct {
        /* TODO: switch to flag and masks. */
#if defined(WORDS_BIGENDIAN) && (WORDS_BIGENDIAN == 1)
        uint8_t  fin     : 1;
        uint8_t  rsv1    : 1;
        uint8_t  rsv2    : 1;
        uint8_t  rsv3    : 1;
        uint8_t  op_code : 4;
#else
        uint8_t  op_code : 4;
        uint8_t  rsv3    : 1;
        uint8_t  rsv2    : 1;
        uint8_t  rsv1    : 1;
        uint8_t  fin     : 1;
#endif /* BIG_ENDIAN */
    };
} ymo_ws_frame_flags_t;

/** Websocket frame */
typedef struct ymo_ws_frame {
    uint64_t  len;     /* TODO: just use top-bit for the masked flag? */
    size_t    parsed;

    /* For incoming messages: */
    uint8_t               masking_key[4] YMO_ATTR_ALIGNED(sizeof(uint32_t));
    ws_parse_state_t      parse_state;
    uint8_t               masked;
    ymo_ws_frame_flags_t  flags;

    /* The following have mutually exclusive lifecycles: */
    union {
        uint8_t  mask_mod;
        uint8_t  len_idx;
    };

} ymo_ws_frame_t;

YMO_ENUM8_TYPEDEF(ws_msg_state) {
    WS_MSG_START,
    WS_MSG_CONTINUE,
} YMO_ENUM8_AS(ws_msg_state_t);

/** Internal structure used to manage a yimmo ws session. */
struct ymo_ws_session {
    /* TODO: can we get rid of embedding the conn in the session? */
    ymo_conn_t*          conn;
    ymo_ws_proto_data_t* p_data;     /* HACK */
    ymo_ws_frame_t       frame_in;
    ymo_bucket_t*        recv_head;
    ymo_bucket_t*        recv_tail;
    ymo_bucket_t*        send_head;
    ymo_bucket_t*        send_tail;

    /* TODO: need state for in + out? (combine in/out/closing into uint8_t?) */
    ws_msg_state_t  msg_state;

    /* TODO: convert to ws_session_state enum? */
    uint8_t  closing;

    /* Buffered messages (if no recv callback is defined, we buffer the
     * whole message, up to WS_MSG_LEN_MAX bytes and deliver that):
     */
    char*   msg;
    char*   msg_end;
    size_t  msg_len;
};

/*---------------------------------------------------------------*
 *  Functions
 *---------------------------------------------------------------*/

/** Create a new WS session. */
ymo_ws_session_t* ymo_ws_session_create(
        ymo_ws_proto_data_t* p_data, ymo_conn_t* conn);

/** Clear and free the given ws session object, including any nested data
 * which has been dynamically allocated. */
void ymo_ws_session_free(ymo_ws_session_t* session);

/** Send without param/state validation (internal protocol use only): */
ymo_status_t ymo_ws_session_send_no_check(
        ymo_ws_session_t* session,
        uint8_t flags,
        ymo_bucket_t* payload);

#endif /* YMO_WS_SESSION_H */



