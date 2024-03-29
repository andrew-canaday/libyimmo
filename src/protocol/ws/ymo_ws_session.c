/*=============================================================================
 * libyimmo: C WS/WebSocket framework
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
 *.
 *
 *===========================================================================*/

#include "yimmo_config.h"
#include "ymo_log.h"
#include "ymo_util.h"
#include "yimmo.h"
#include "ymo_alloc.h"
#include "core/ymo_conn.h"
#include "ymo_ws_session.h"

#define YMO_SESSION_MEM_WS_TRACE 0
#if defined(YMO_SESSION_MEM_WS_TRACE) && YMO_SESSION_MEM_WS_TRACE == 1
# define SESSION_MEM_WS_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__);
# define SESSION_MEM_WS_TRACE_UUID(fmt, ...) ymo_log_trace_uuid(fmt, __VA_ARGS__);
#else
# define SESSION_MEM_WS_TRACE(fmt, ...)
# define SESSION_MEM_WS_TRACE_UUID(fmt, ...)
#endif /* YMO_SESSION_MEM_WS_TRACE */


ymo_ws_session_t* ymo_ws_session_create(
        ymo_ws_proto_data_t* p_data, ymo_conn_t* conn)
{
    ymo_ws_session_t* session = YMO_NEW0(ymo_ws_session_t);
    if( session ) {
        session->conn = conn;
        session->p_data = p_data;
    }
    return session;
}


ymo_status_t ymo_ws_session_alloc_frame(
        ymo_ws_session_t* session, size_t len)
{
    if( len > YMO_WS_FRAME_MAX ) {
        SESSION_MEM_WS_TRACE("Requested frame size (%zu) exceeds max (%zu)",
                len, YMO_WS_FRAME_MAX);
        return ENOBUFS;
    }

    if( session->frame_in.buffer ) {
        if( session->frame_in.buf_len >= len ) {
            SESSION_MEM_WS_TRACE(
                    "Existing frame size (%zu) sufficient for request (%zu)",
                    session->frame_in.buf_len, len);
            return YMO_OKAY;
        }

        SESSION_MEM_WS_TRACE(
                "Freeing previous frame (%zu)",
                session->frame_in.buf_len);
        YMO_FREE(session->frame_in.buffer);
        session->frame_in.buffer = NULL;
        session->frame_in.buf_len = 0;
    }

    SESSION_MEM_WS_TRACE(
            "%zu bytes requested. Allocating %zu",
            len, YMO_MAX(YMO_WS_FRAME_MIN, len));
    len = YMO_MAX(YMO_WS_FRAME_MIN,len);
    session->frame_in.buffer = YMO_ALLOC(len);
    if( session->frame_in.buffer ) {
        session->frame_in.buf_len = len;
        return YMO_OKAY;
    }
    return ENOMEM;
}


void ymo_ws_session_free(ymo_ws_session_t* session)
{
    if( session ) {
        if( session->send_head ) {
            ymo_bucket_free_all(session->send_head);
        }

        if( session->msg ) {
            YMO_FREE(session->msg);
        }

        if( session->frame_in.buffer ) {
            YMO_FREE(session->frame_in.buffer);
        }
        YMO_DELETE(ymo_ws_session_t, session);
    }
}


#define WS_HDR_LEN(n) (n+2)

static char* gen_ws_msg_hdr(size_t* hdr_len, uint8_t flag, size_t len)
{
    char* hdr_data = NULL;
    uint8_t len0;
    uint8_t ext_len;

    if( len <= 125 ) {
        len0 = (uint8_t)len;
        ext_len = 0;
    } else if( len <= 0xffff ) {
        len0 = 126;
        ext_len = 2;
    } else {
        len0 = 127;
        ext_len = 8;
    }

    hdr_data = YMO_ALLOC(WS_HDR_LEN(ext_len));
    if( hdr_data ) {
        *hdr_len = WS_HDR_LEN(ext_len);
        hdr_data[0] = (char)(flag);
        hdr_data[1] = (char)(len0);

        switch( ext_len ) {
            case 2:
                hdr_data[2] = (len >> 8) & 0xFF;
                hdr_data[3] = len & 0xFF;
                break;

            case 8:
                hdr_data[2] = (len >> (8*7)) & 0xFF;
                hdr_data[3] = (len >> (8*6)) & 0xFF;
                hdr_data[4] = (len >> (8*5)) & 0xFF;
                hdr_data[5] = (len >> (8*4)) & 0xFF;

                hdr_data[6] = (len >> (8*3)) & 0xFF;
                hdr_data[7] = (len >> (8*2)) & 0xFF;
                hdr_data[8] = (len >> 8) & 0xFF;
                hdr_data[9] = (len) & 0xFF;
                break;
            default:
                break;
        }
    }
    return hdr_data;
}


ymo_status_t ymo_ws_session_send_no_check(
        ymo_ws_session_t* session,
        uint8_t flags,
        ymo_bucket_t* payload)
{
    size_t hdr_len = 0;
    size_t msg_len = ymo_bucket_len_all(payload);
    char* hdr_data = gen_ws_msg_hdr(&hdr_len, flags, msg_len);
    if( !hdr_data ) {
        return errno;
    }

    ymo_bucket_t* hdr_out = ymo_bucket_create(
            session->send_tail, payload,
            hdr_data, hdr_len, hdr_data, hdr_len);

    if( !session->send_head ) {
        session->send_head = hdr_out;
    }

    session->send_tail = ymo_bucket_tail(payload);
    ymo_conn_tx_enable(session->conn, 1);
    return YMO_OKAY;
}


ymo_status_t ymo_ws_session_send(
        ymo_ws_session_t* session,
        uint8_t flags,
        ymo_bucket_t* payload)
{
    switch( flags & YMO_WS_MASK_OP ) {
        case YMO_WS_OP_TEXT:
        case YMO_WS_OP_BINARY:
        case YMO_WS_OP_CONTINUATION:
        case YMO_WS_OP_CLOSE:
        case YMO_WS_OP_PING:
        case YMO_WS_OP_PONG:
            break;
        default:
            return EINVAL;
    }

    /* Most likely scenario: */
    if( session->state == WS_SESSION_CONNECTED ) {
        return ymo_ws_session_send_no_check(session, flags, payload);
    }

    /* Else, tell them why it failed: */
    ymo_status_t err_status;
    switch( session->state ) {
        case WS_SESSION_CLOSE_RECEIVED:
        case WS_SESSION_EXPECT_CLOSE:
            err_status = EADDRNOTAVAIL;
            break;
        case WS_SESSION_CLOSED:
            err_status = EPIPE;
            break;
        case WS_SESSION_ERROR:
            err_status = ECONNABORTED;
            break;
    }

    return err_status;
}


void ymo_ws_session_set_userdata(ymo_ws_session_t* session, void* user_data)
{
    session->user_data = user_data;
}


void* ymo_ws_session_get_userdata(ymo_ws_session_t* session)
{
    return session->user_data;
}

