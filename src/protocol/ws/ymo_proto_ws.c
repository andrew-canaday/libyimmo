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

#include "ymo_config.h"

#include "ymo_alloc.h"
#include "ymo_blalloc.h"
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <signal.h>

#include "yimmo.h"
#include "ymo_log.h"
#include "core/ymo_net.h"
#include "core/ymo_server.h"
#include "core/ymo_conn.h"
#include "ymo_alloc.h"
#include "ymo_util.h"
#include "ymo_ws.h"
#include "ymo_proto_ws.h"
#include "ymo_http.h"
#include "ymo_ws_parse.h"
#include "ymo_ws_session.h"

/* Buffering recv_cb used when YMO_WS_SERVER_BUFFERED is set: */
static ymo_status_t ws_buffer_msg(
        ymo_ws_session_t* session,
        void* user_data,
        uint8_t flags,
        const char* msg,
        size_t len);


/*---------------------------------------------------------------*
 *  Yimmo WS Protocol:
 *---------------------------------------------------------------*/
static ymo_proto_vt_t ymo_default_ws_proto = {
    .init_cb = &ymo_proto_ws_init,
    .cleanup_cb = &ymo_proto_ws_cleanup,
    .conn_init_cb = &ymo_proto_ws_conn_init,
    .conn_ready_cb = &ymo_proto_ws_conn_ready,
    .read_cb = &ymo_proto_ws_read,
    .write_cb = &ymo_proto_ws_write,
    .conn_cleanup_cb = &ymo_proto_ws_conn_cleanup,
};

struct ymo_ws_proto_data {
    ymo_ws_connect_cb_t  connect_cb;
    ymo_ws_recv_cb_t     recv_cb;
    ymo_ws_recv_cb_t     msg_cb;
    ymo_ws_close_cb_t    close_cb;
};

ymo_proto_t* ymo_proto_ws_create(
        ymo_ws_server_flags_t flags,
        ymo_ws_connect_cb_t connect_cb,
        ymo_ws_recv_cb_t recv_cb,
        ymo_ws_close_cb_t close_cb)
{
    /* Allocate and initialize the protocol object: */
    ymo_proto_t* ws_proto = YMO_NEW0(ymo_proto_t);
    if( !ws_proto ) {
        goto proto_bail;
    }
    ws_proto->name = "WS";
    ws_proto->vtable = ymo_default_ws_proto;

    /* Allocate the nested private data: */
    ymo_ws_proto_data_t* data = YMO_NEW0(ymo_ws_proto_data_t);
    if( !data ) {
        goto proto_free_and_bail;
    }
    ws_proto->data = (void*)data;

    /* Initialize the nested private data: */
    data->connect_cb = connect_cb;

    if( flags & YMO_WS_SERVER_BUFFERED ) {
        data->recv_cb = &ws_buffer_msg;
        data->msg_cb = recv_cb;
    } else {
        data->recv_cb = recv_cb;
    }
    data->close_cb = close_cb;
    return ws_proto;

    /*
proto_free_data_and_bail:
    YMO_DELETE(ymo_ws_proto_data_t, data);
    */
proto_free_and_bail:
    YMO_DELETE(ymo_proto_t, ws_proto);
proto_bail:
    return NULL;
}

/*---------------------------------------------------------------*
 *  Yimmo WS Protocol Init Callback:
 *---------------------------------------------------------------*/
ymo_status_t ymo_proto_ws_init(ymo_proto_t* proto, ymo_server_t* server)
{
    return YMO_OKAY;
}


/*---------------------------------------------------------------*
 *  Yimmo WS Protocol Cleanup Callback:
 *---------------------------------------------------------------*/
void ymo_proto_ws_cleanup(ymo_proto_t* proto, ymo_server_t* server)
{
    return;
}


/*---------------------------------------------------------------*
 *  Yimmo WS Protocol Client Init Callback:
 *---------------------------------------------------------------*/
void* ymo_proto_ws_conn_init(void* proto_data, ymo_conn_t* conn)
{
    ymo_ws_session_t* ws_session = ymo_ws_session_create(
            proto_data, conn);
    return (void*)ws_session;
}


/*---------------------------------------------------------------*
 *  Yimmo WS Protocol Client Ready Callback:
 *---------------------------------------------------------------*/
ymo_status_t ymo_proto_ws_conn_ready(
        void* proto_data, ymo_conn_t* conn, void* conn_data)
{
    ymo_ws_proto_data_t* p_data = proto_data;
    ymo_ws_session_t* ws_session = conn_data;
    if( p_data->connect_cb ) {
        ymo_conn_tx_enable(conn, 1);
        ymo_conn_rx_enable(conn, 1);
        return p_data->connect_cb(ws_session);
    }
    return YMO_OKAY;
}


/*---------------------------------------------------------------*
 *  Yimmo WS Protocol Client Cleanup Callback:
 *---------------------------------------------------------------*/
void ymo_proto_ws_conn_cleanup(
        void* proto_data, ymo_conn_t* conn, void* conn_data)
{
    if( conn_data ) {
        ymo_ws_session_free((ymo_ws_session_t*)conn_data);
    }
    return;
}


/*---------------------------------------------------------------*
 *  Yimmo WS Protocol Read Callback:
 *---------------------------------------------------------------*/

static ymo_status_t ws_buffer_msg(
        ymo_ws_session_t* session,
        void* user_data,
        uint8_t flags,
        const char* msg,
        size_t len)
{
    ymo_status_t r_msg = YMO_OKAY;
    if( !len ) {
        return r_msg;
    }

    if( !session->msg ) {
        session->msg = session->msg_end = YMO_ALLOC(YMO_WS_MSG_LEN_MAX);

        if( !session->msg ) {
            ymo_log_warning("Out of memory allocating WS message buffer");
            return ENOMEM;
        }

        session->msg_len = 0;
    }

    /* Bail if we're out of buffer space for this message. */
    if( len > YMO_WS_MSG_LEN_MAX - session->msg_len ) {
        ymo_log_warning("Incoming message exceeds max WS message buffer size");
        return EFBIG;
    }

    memcpy(session->msg_end, msg, len);
    session->msg_end += len;

    /* If this was the last of the message, go ahead and deliver it to the
     * msg callback and reset the buffer:
     */
    if( flags & YMO_WS_FLAG_FIN ) {
        r_msg = session->p_data->msg_cb(
                session, session->user_data, flags, session->msg, session->msg_len);
        session->msg_end = session->msg;
        session->msg_len = 0;
    }

    return r_msg;
}

static ymo_status_t
issue_ws_callbacks(
        ymo_ws_recv_cb_t recv_cb,
        ymo_ws_session_t* session)
{
    ymo_status_t status = ENOENT;
    ymo_bucket_t* p = session->recv_head;

    while( p ) {
        status = recv_cb(
                session,
                session->user_data,
                session->frame_in.flags.packed,
                p->data, p->len);
        if( status != YMO_OKAY ) {
            ymo_log_debug("Callback failure: %i", status);
            goto callback_bail;
        }

        p = p->next;
    }

callback_bail:
    ymo_bucket_free_all(session->recv_head);
    session->recv_head = session->recv_tail = NULL;
    return status;
}


static ymo_status_t
ws_echo_cb(
        ymo_ws_session_t* session,
        void* user_data,
        uint8_t flags,
        const char* data,
        size_t len)
{
    ymo_ws_frame_flags_t frame_flags = { .packed = flags };
    ymo_status_t status = YMO_OKAY;
    if( data && len ) {
        if( frame_flags.op_code == YMO_WS_OP_PING ) {
            frame_flags.op_code = YMO_WS_OP_PONG;
        }

        /** TODO: Introduce input + output message state (what does this mean?). */
        status = ymo_ws_session_send_no_check(
                session, frame_flags.packed, YMO_BUCKET_FROM_CPY(data, len));
    }
    return status;
}


/* WS Protocol read callback: */
ssize_t ymo_proto_ws_read(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        char* recv_buf,
        size_t len)
{
    ymo_ws_session_t* session = conn_data;
    ymo_ws_proto_data_t* p_data = proto_data;
    char* parse_buf = recv_buf;
    do {
        ssize_t n = 0;
        switch( session->frame_in.parse_state ) {
            case WS_PARSE_OP:
                n = ymo_ws_parse_op(session, parse_buf, len);
                break;
            case WS_PARSE_LEN:
                n = ymo_ws_parse_len(session, parse_buf, len);
                break;
            case WS_PARSE_LEN_EXTENDED:
                n = ymo_ws_parse_len_ext(session, parse_buf, len);
                break;
            case WS_PARSE_MASKING_KEY:
                n = ymo_ws_parse_masking_key(session, parse_buf, len);
                break;
            case WS_PARSE_PAYLOAD:
                n = ymo_ws_parse_payload(session, parse_buf, len);
                break;
            case WS_PARSE_COMPLETE:
                goto ws_parse_complete;
                break;
            default:
                errno = EBADMSG;
                return -1;
        }

        if( n >= 0 ) {
            parse_buf += n;
            len -= n;
            ymo_log_trace("%i bytes parsed (remain=%lu)", (int)n, len);
        } else {
            /* Errno has been set by the last parse function: */
            return -1;
        }
    } while( len );

    /* NOTE: Check to see if parsing is complete. If there is no read data
     * after the end of a complete message, the WS_PARSE_COMPLETE case above
     * will not be reached, otherwise. */
    if( session->frame_in.parse_state == WS_PARSE_COMPLETE ) {
ws_parse_complete:
        {
            ymo_log_trace("WS frame parse complete. Flags: 0x%x; Fin: 0x%x, Op: 0x%x",
                    session->frame_in.flags.packed,
                    session->frame_in.flags.fin,
                    session->frame_in.flags.op_code);
            ymo_log_trace("ALSO: WS frame parse complete. Flags: 0x%x; Fin: 0x%x, Op: 0x%x",
                    session->frame_in.flags.packed,
                    session->frame_in.flags.packed & YMO_WS_FLAG_FIN,
                    session->frame_in.flags.packed & YMO_WS_MASK_OP);
            ymo_status_t cb_status = YMO_OKAY;

            /* Standard messages: invoke user callback: */
            switch( session->frame_in.flags.op_code ) {
                /* --- USER callback: --- */
                case YMO_WS_OP_CONTINUATION:
                /* fallthrough */
                case YMO_WS_OP_TEXT:
                /* fallthrough */
                case YMO_WS_OP_BINARY:
                    ymo_log_trace("Invoking callback: %s", "user callback");
                    cb_status = issue_ws_callbacks(p_data->recv_cb, session);
                    break;

                /* --- SERVER callback: --- */
                case YMO_WS_OP_CLOSE:
                    session->closing = 1;
                    if( p_data->close_cb ) {
                        p_data->close_cb(session, session->user_data);
                    }
                    break;
                case YMO_WS_OP_PING:
                    ymo_log_trace("Invoking callback: %s", "ws_echo_cb");
                    cb_status = issue_ws_callbacks(ws_echo_cb, session);
                    break;

                /* --- Ignored: ---- */
                case YMO_WS_OP_PONG:
                    cb_status = YMO_OKAY;
                    /* Ignore PONG frames */
                    break;
            }

            /* Reset frame in on success; else bail: */
            if( cb_status == YMO_OKAY ) {
                memset(&(session->frame_in), 0, sizeof(ymo_ws_frame_t));
            } else {
                errno = cb_status;
                return -1;
            }
        }
    }

    return (parse_buf - recv_buf);
}


/*---------------------------------------------------------------*
 *  Yimmo WS Protocol Write Callback:
 *---------------------------------------------------------------*/
ymo_status_t ymo_proto_ws_write(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        int socket)
{
    ymo_status_t status = YMO_OKAY;
    ymo_ws_session_t* session = conn_data;

    if( !session->send_head ) {
        ymo_conn_tx_enable(conn, 0);
        return YMO_OKAY;
    }

    status = ymo_send_buckets(socket, &session->send_head);

    /* If all data was sent okay, clear the send queue: */
    if( status == YMO_OKAY ) {
        ymo_log_trace("All data sent okay for: %i", socket);
        session->send_head = session->send_tail = NULL;

        if( !session->closing ) {
            ymo_conn_tx_enable(conn, 0);
        } else {
            ymo_log_debug("Close frame sent. Terminating: %i", socket);
            ymo_conn_close(conn);
        }
    }
    return status;
}


/*---------------------------------------------------------------*
 *  Yimmo HTTP->WS Upgrade handler:
 *---------------------------------------------------------------*/
ymo_http_upgrade_handler_t* ymo_ws_http_upgrade_handler(ymo_proto_t* proto)
{
    ymo_http_upgrade_handler_t* upgrade_handler = YMO_NEW(ymo_http_upgrade_handler_t);
    if( upgrade_handler ) {
        upgrade_handler->cb = &ymo_ws_upgrade_cb;
        upgrade_handler->proto_new = proto;
    }
    return upgrade_handler;
}


static ymo_status_t
ymo_ws_issue_101(
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        const char* key)
{
    /* Combine key and GUID: */
    size_t input_len = strlen(key) + strlen(WS_GUID);
    unsigned char* input = YMO_ALLOC(input_len + 1);
    sprintf((char*)input, "%s%s", key, WS_GUID);

    /* Compute the SHA-1: */
    unsigned char hashed_hdr[20];
    SHA1(input, input_len, hashed_hdr);
    YMO_FREE(input);

    char* accept_hdr = ymo_blalloc(request->ws, sizeof(char), YMO_BASE64_LEN(20));
    if( !accept_hdr ) {
        return ENOMEM;
    }

    /* TODO: Check return value? */
    ymo_base64_encode(accept_hdr, hashed_hdr, 20);

    /* Send off the request: */
    ymo_http_response_set_status(response, 101);
    ymo_http_response_set_header(response, "Server", "ymo_test_server");
    ymo_http_response_set_header(response, "Upgrade", "websocket");
    ymo_http_response_set_header(response, "Connection", "upgrade");
    ymo_http_response_set_header(response, "Sec-Websocket-Accept", accept_hdr);
    ymo_http_response_set_flags(response, YMO_HTTP_FLAG_REQUEST_KEEPALIVE);
    ymo_http_response_finish(response);
    return YMO_OKAY;
}

static ymo_status_t ymo_ws_check_connection_hdr(
        ymo_http_request_t* request, const char* conn_hdr)
{
    static const char* sep = " ,\n\t";

    ymo_status_t r = EINVAL;
    if( conn_hdr ) {
        size_t hdr_len = strlen(conn_hdr);
        char* conn_str = ymo_blalloc(request->ws, sizeof(char), hdr_len+1);
        if( !conn_str ) {
            return ENOMEM;
        }

        strcpy(conn_str, conn_hdr);
        char* saveptr = NULL;
        char* hdr_val = NULL;
        for( hdr_val = strtok_r(conn_str, sep, &saveptr); hdr_val;
             hdr_val = strtok_r(NULL, sep, &saveptr))
        {
            if( !strcasecmp(hdr_val, "upgrade") ) {
                r = YMO_OKAY;
                break;
            }
        }
    }
    return r;
}

ymo_http_upgrade_status_t ymo_ws_upgrade_cb(
        const char* hdr_upgrade,
        ymo_http_request_t* request,
        ymo_http_response_t* response)
{
    /* Bail immediately, if this isn't an attempted ws upgrade: */
    if( strcasecmp(hdr_upgrade, "websocket") ) {
        return YMO_HTTP_UPGRADE_NOPROTO;
    }

    ymo_status_t resp_status;
    ymo_http_upgrade_status_t upgrade_status;

    const char* host = ymo_http_hdr_table_get(
            &request->headers, "host");
    const char* connection = ymo_http_hdr_table_get(
            &request->headers, "connection");
    const char* sec_ws_key = ymo_http_hdr_table_get(
            &request->headers, "sec-websocket-key");
    const char* origin = ymo_http_hdr_table_get(
            &request->headers, "origin");
    const char* sec_ws_proto = ymo_http_hdr_table_get(
            &request->headers, "sec-websocket-protocol");
    const char* sec_ws_version = ymo_http_hdr_table_get(
            &request->headers, "sec-websocket-version");

    long ws_version = 0;
    if( sec_ws_version ) {
        ws_version = strtol(sec_ws_version, NULL, 10);
    }

    /* Error 400, on invalid header data (RFC6455 4.2.1.): */
    if( strcasecmp(request->method, "get")
#ifdef TODO_REQUEST_FLAGS_NOT_EXPOSED
        || !(request->flags & YMO_HTTP_FLAG_VERSION_1_1)
#endif
        || (!host)
        || (ymo_ws_check_connection_hdr(request, connection) != YMO_OKAY)
        || (!sec_ws_key)
        || (ws_version != RFC_6455)
        ) {
        ymo_log_debug("Bad \"%s\" upgrade request for %s", hdr_upgrade, request->uri);
        ymo_log_debug("\thost: %s", host);
        ymo_log_debug("\tconnection: %s", connection);
        ymo_log_debug("\torigin: %s", origin);
        ymo_log_debug("\tsec_ws_key: %s", sec_ws_key);
        ymo_log_debug("\tsec_ws_proto: %s", sec_ws_proto);
        ymo_log_debug("\tsec_ws_version: %s", sec_ws_version);
        errno = EPROTONOSUPPORT;
        upgrade_status = YMO_HTTP_UPGRADE_ERROR;
        resp_status = ymo_http_response_issue(response,
                YMO_HTTP_UPGRADE_REQUIRED);
    } else {
        /* TODO: Optional "origin" header callback. */
        ymo_log_trace("\thost: %s", host);
        ymo_log_trace("\tconnection: %s", connection);
        ymo_log_trace("\torigin: %s", origin);
        ymo_log_trace("\tsec_ws_key: %s", sec_ws_key);
        ymo_log_trace("\tsec_ws_proto: %s", sec_ws_proto);
        ymo_log_trace("\tsec_ws_version: %s", sec_ws_version);
        upgrade_status = YMO_HTTP_UPGRADE_HANDLED;
        resp_status = ymo_ws_issue_101(request, response, sec_ws_key);
    }

    if( resp_status != YMO_OKAY ) {
        errno = resp_status;
        upgrade_status = YMO_HTTP_UPGRADE_ERROR;
    }
    return upgrade_status;
}




