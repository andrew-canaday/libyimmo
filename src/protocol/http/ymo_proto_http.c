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
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <errno.h>

#include "yimmo.h"
#include "ymo_log.h"
#include "core/ymo_net.h"
#include "core/ymo_server.h"
#include "core/ymo_conn.h"
#include "ymo_alloc.h"
#include "ymo_proto_http.h"
#include "ymo_http_session.h"
#include "ymo_http_parse.h"
#include "ymo_http_exchange.h"
#include "ymo_http_response.h"

#define YMO_HTTP_TRACE_PROTO 0
#if defined(YMO_HTTP_TRACE_PROTO) && YMO_HTTP_TRACE_PROTO == 1
#define HTTP_PROTO_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__);

/* HACK HACK HACK: */
static const char* state_names[] = {
    "CONNECTED",
    "REQUEST_METHOD",
    "REQUEST_URI_PATH",
    "REQUEST_QUERY",
    "REQUEST_FRAGMENT",
    "REQUEST_VERSION",
    "CRLF",
    "HEADER_NAME",
    "HEADER_VALUE_LEADING_SPACE",
    "HEADER_VALUE",
    "EXPECT",
    "BODY_CHUNK_HEADER",
    "BODY",
    "BODY_CHUNK_TRAILER",
    "COMPLETE",
};

#define HTTP_PARSE_STATE_NAME(x) state_names[x]

#else
#define HTTP_PROTO_TRACE(fmt, ...)
#define HTTP_PARSE_TRACE(fmt, ...)
#endif /* YMO_HTTP_TRACE_PROTO */

/*---------------------------------------------------------------*
 *  Yimmo HTTP Protocol:
 *---------------------------------------------------------------*/
static ymo_proto_vt_t ymo_default_http_proto = {
    .init_cb = &ymo_proto_http_init,
    .cleanup_cb = &ymo_proto_http_cleanup,
    .conn_init_cb = &ymo_proto_http_conn_init,
    .read_cb = &ymo_proto_http_read,
    .write_cb = &ymo_proto_http_write,
    .conn_cleanup_cb = &ymo_proto_http_conn_cleanup,
};

static ymo_status_t buffer_body_cb(
        ymo_http_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        const char* data,
        size_t len,
        void* user)
{
    if( !request->body ) {
        /* If we're buffering the body, reset content length to zero here
         * and increment it as we accumulate (the parser is tracking using
         * request->body_remain).
         */
        request->content_length = 0;
        /* HACK: make the limit configurable! */
        if( request->content_length <= YMO_HTTP_MAX_BODY ) {
            request->body = YMO_ALLOC(YMO_HTTP_MAX_BODY);
        }

        if( !request->body ) {
            ymo_log_warning(
                    "Failed to allocate %i bytes for body buffer.",
                    YMO_HTTP_MAX_BODY);
            return ENOMEM;
        }
    }

    size_t buf_remain = YMO_HTTP_MAX_BODY - request->body_received;
    if( len > buf_remain ) {
        ymo_log_warning(
                "Not enough body buffer space remaining "
                "(buffer: %zu; payload: %zu).",
                buf_remain, len);
        return ENOMEM;
    }

    char* body_end = request->body + request->body_received;
    request->body_received += len;
    request->content_length += len;
    memcpy(body_end, data, len);
    return YMO_OKAY;
}


ymo_proto_t* ymo_proto_http_create(
        ymo_http_session_init_cb_t session_init,
        ymo_http_cb_t http_callback,
        ymo_http_header_cb_t header_callback,
        ymo_http_body_cb_t body_callback,
        ymo_http_session_cleanup_cb_t session_cleanup,
        void* data)
{
    /* Allocate and initialize the protocol object: */
    ymo_proto_t* http_proto = YMO_NEW0(ymo_proto_t);
    if( !http_proto ) {
        goto proto_bail;
    }
    http_proto->name = "HTTP";
    http_proto->vtable = ymo_default_http_proto;

    /* Allocate the nested private http_data: */
    ymo_http_proto_data_t* http_data = YMO_NEW(ymo_http_proto_data_t);
    if( !http_data ) {
        goto proto_free_and_bail;
    }
    http_data->data = data;
    http_proto->data = (void*)http_data;

    /* Initialize the nested private http_data: */
    http_data->session_init = session_init;
    http_data->http_cb = http_callback;
    http_data->header_cb = header_callback;
    if( body_callback ) {
        http_data->body_cb = body_callback;
    } else {
        http_data->body_cb = &buffer_body_cb;
    }
    http_data->session_cleanup = session_cleanup;
    http_data->upgrade_handler = NULL;
    return http_proto;

proto_free_and_bail:
    YMO_DELETE(ymo_proto_t, http_proto);
proto_bail:
    return NULL;
}


ymo_status_t
ymo_http_add_upgrade_handler(
        ymo_proto_t* proto, ymo_http_upgrade_handler_t* upgrade_handler)
{
    if( upgrade_handler == NULL ) {
        return EINVAL;
    }

    ymo_http_proto_data_t* proto_data = \
        (ymo_http_proto_data_t*)proto->data;

    ymo_http_upgrade_chain_t** upgrade_p = &(proto_data->upgrade_handler);

    while( *upgrade_p )
    {
        upgrade_p = &((*upgrade_p)->next);
    }

    ymo_http_upgrade_chain_t* upgrade_n = YMO_NEW0(ymo_http_upgrade_chain_t);
    (*upgrade_p) = upgrade_n;

    ymo_status_t r_val;
    if( upgrade_n ) {
        upgrade_n->cb = upgrade_handler->cb;
        upgrade_n->proto = upgrade_handler->proto_new;
        r_val = YMO_OKAY;
    } else {
        r_val = ENOMEM;
    }

    YMO_DELETE(ymo_http_upgrade_handler_t, upgrade_handler);
    return r_val;
}


/*---------------------------------------------------------------*
 *  HTTP 2.0:
 *---------------------------------------------------------------*/
/** For the time being: indicate lack of HTTP/2.0 support by
 * responding with HTTP/1.1.
 *
 * TODO HTTP 2.0 support!
 */
ymo_http_upgrade_status_t ymo_http2_not_available_upgrade_cb(
        const char* hdr_upgrade,
        ymo_http_request_t* request,
        ymo_http_response_t* response)
{
    if( strcasecmp(hdr_upgrade, "h2c") ) {
        errno = EPROTONOSUPPORT;
        return YMO_HTTP_UPGRADE_NOPROTO;
    }

    ymo_log_debug("Downgrading HTTP/2 (\"%s\") \"%s %s\" to HTTP/1.1",
            hdr_upgrade, request->method, request->uri);
    return YMO_HTTP_UPGRADE_IGNORE;
}


ymo_http_upgrade_handler_t* ymo_http2_no_upgrade_handler()
{
    ymo_http_upgrade_handler_t* upgrade_handler = YMO_NEW(ymo_http_upgrade_handler_t);
    if( upgrade_handler ) {
        upgrade_handler->cb = &ymo_http2_not_available_upgrade_cb;
        upgrade_handler->proto_new = NULL;
    }
    return upgrade_handler;
}


/*---------------------------------------------------------------*
 *  HTTP Request Handler:
 *---------------------------------------------------------------*/
static ymo_http_upgrade_status_t ymo_http_check_upgrade(
        ymo_conn_t* conn,
        ymo_http_proto_data_t* proto_data,
        const char* method,
        const char* uri,
        const char* hdr_upgrade,
        ymo_http_exchange_t* exchange,
        ymo_http_response_t* response)
{
    ymo_log_debug("Got upgrade exchange: \"%s\".", hdr_upgrade);
    ymo_http_upgrade_status_t status = YMO_HTTP_UPGRADE_NOPROTO;
    ymo_http_upgrade_chain_t* upgrade = proto_data->upgrade_handler;
    while( upgrade )
    {
        status = upgrade->cb(hdr_upgrade, &(exchange->request), response);

        switch( status ) {
            case YMO_HTTP_UPGRADE_HANDLED:
                /* Upgrade handled; inform caller: */
                ymo_log_debug("Performing upgrade to %s (%p)",
                        hdr_upgrade, (void*)upgrade->proto);
                response->proto_new = upgrade->proto;
                YMO_STMT_ATTR_FALLTHROUGH();
            case YMO_HTTP_UPGRADE_IGNORE:
                YMO_STMT_ATTR_FALLTHROUGH();
            case YMO_HTTP_UPGRADE_ERROR:
                goto return_http_upgrade_status;
                break;

            case YMO_HTTP_UPGRADE_NOPROTO:
                /* If this handler doesn't support, try the next: */
                upgrade = upgrade->next;
                break;

            /* An error occurred; inform caller: */
            default:
                errno = EINVAL;
                return YMO_HTTP_UPGRADE_ERROR;
        }
    }

return_http_upgrade_status:
    return status;
}


/* Handle yimmo URI's internally, else hand the exchange over to the user. */
static ymo_status_t ymo_http_handler(
        ymo_server_t* server,
        ymo_conn_t* conn,
        ymo_http_proto_data_t* proto_data,
        ymo_http_session_t* http_session,
        ymo_http_exchange_t* exchange)
{
    ymo_log_debug("Handling exchange for %s", exchange->request.uri);
    ymo_status_t status = YMO_OKAY;
    ymo_http_response_t* response = exchange->response;

    /* Is this an upgrade exchange? */
    ymo_http_upgrade_status_t upgrade_status = YMO_HTTP_UPGRADE_IGNORE;
    if( exchange->request.flags & YMO_HTTP_FLAG_UPGRADE ) {
        const char* hdr_upgrade = NULL;
        hdr_upgrade = ymo_http_hdr_table_get_id(
                &exchange->request.headers, HDR_ID_UPGRADE);
        HTTP_PROTO_TRACE("Got upgrade exchange: \"%s\"", hdr_upgrade);
        /* Check for upgrade handlers: */
        upgrade_status = ymo_http_check_upgrade(
                conn, proto_data,
                exchange->request.method, exchange->request.uri, hdr_upgrade,
                exchange, response);

        switch( upgrade_status ) {
            case YMO_HTTP_UPGRADE_IGNORE:
                goto issue_standard_callback;
                break;
            case YMO_HTTP_UPGRADE_HANDLED:
                goto handle_callback_result;
                break;
            case YMO_HTTP_UPGRADE_ERROR:
                status = errno;
                goto handle_callback_result;
                break;
            case YMO_HTTP_UPGRADE_NOPROTO:
                /* TODO: 4xx/5xx should be determined by type of error...
                 * For now 400 error on bad upgrade. */
                status = ymo_http_response_issue(response,
                        YMO_HTTP_UPGRADE_REQUIRED);
                goto handle_callback_result;
                break;
            default:
                break;
        }
    }


issue_standard_callback:
    /* Issue standard HTTP callback: */
    status = proto_data->http_cb(
            http_session, &(exchange->request),
            response, http_session->user_data);

handle_callback_result:
    if( status == YMO_OKAY ) {
        ymo_http_session_add_response(http_session, response);

        if( response->flags & YMO_HTTP_FLAG_REQUEST_KEEPALIVE ) {
            HTTP_PROTO_TRACE("Resetting http exchange: %p", (void*)exchange);
            ymo_http_exchange_reset(exchange);
        }
    } else if( YMO_IS_BLOCKED(status) ) {
        /* Okay, they held onto the exchange, so don't reuse it: */
        response->exchange = http_session->exchange;
        http_session->exchange = NULL;
        ymo_http_session_add_response(http_session, response);
        return status;
    } else {
        /* TODO, we need to figure out *why* we bailed here and then
         * decide if it's 4xx/5xx or just terminate conn. */
        goto bail_free_response;
    }

    /* All good: */
    return status;

bail_free_response:
    HTTP_PROTO_TRACE("Error: freeing response %p", (void*)response);
    ymo_http_response_free(response);
    return status;
}


/*---------------------------------------------------------------*
 *  Yimmo HTTP Protocol Init Callback:
 *---------------------------------------------------------------*/
ymo_status_t ymo_proto_http_init(ymo_proto_t* proto, ymo_server_t* server)
{
    return YMO_OKAY;
}


/*---------------------------------------------------------------*
 *  Yimmo HTTP Protocol Cleanup Callback:
 *---------------------------------------------------------------*/
void ymo_proto_http_cleanup(ymo_proto_t* proto, ymo_server_t* server)
{
    return;
}


/*---------------------------------------------------------------*
 *  Yimmo HTTP Protocol Client Init Callback:
 *---------------------------------------------------------------*/
void* ymo_proto_http_conn_init(void* proto_data, ymo_conn_t* conn)
{
    ymo_http_proto_data_t* http_proto_data = proto_data;
    ymo_http_session_t* session = ymo_http_session_create(conn);
    if( session && http_proto_data->session_init ) {
        ymo_status_t rc_init = http_proto_data->session_init(
                http_proto_data->data, session);

        if( rc_init ) {
            ymo_http_session_free(session);
            session = NULL;
        }
    }
    return session;
}


/*---------------------------------------------------------------*
 *  Yimmo HTTP Protocol Client Cleanup Callback:
 *---------------------------------------------------------------*/
void ymo_proto_http_conn_cleanup(
        void* proto_data, ymo_conn_t* conn, void* conn_data)
{
    ymo_http_proto_data_t* http_proto_data = proto_data;
    ymo_http_session_t* session = conn_data;
    if( http_proto_data->session_cleanup ) {
        http_proto_data->session_cleanup(
                http_proto_data->data, session, session->user_data);
    }
    ymo_http_session_free(session);
    return;
}


/*---------------------------------------------------------------*
 *  Yimmo HTTP Protocol Read Callback:
 *---------------------------------------------------------------*/
/* HTTP Protocol read callback: */
ssize_t ymo_proto_http_read(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        char* recv_buf,
        size_t len)
{
    ymo_http_session_t* http_session = conn_data;
    ymo_http_proto_data_t* http_proto_data = proto_data;
    const char* parse_buf = recv_buf;
    ymo_http_exchange_t* exchange = NULL;

    /* Make sure we've got structures to read into: */
    ymo_status_t status = ymo_http_session_is_read_ready(http_session);
    if( status != YMO_OKAY ) {
        ymo_log_warning("Unable to prepare conn for reading: %s (%i)",
                strerror(status), status);
        errno = status;
        return -1;
    }

    exchange = http_session->exchange;

/* http_parse_resume: */
    do {
        ssize_t n = 0;
        HTTP_PROTO_TRACE("Send to parser:\n"
                "State: %s (next: %s); remain: %zu; buff:\n"
                "---\n%*s\n...",
                HTTP_PARSE_STATE_NAME(exchange->state),
                HTTP_PARSE_STATE_NAME(exchange->next_state),
                len, (int)len, parse_buf);
        switch( exchange->state ) {
            case HTTP_STATE_CONNECTED:
                exchange->state = HTTP_STATE_REQUEST_METHOD;
                YMO_STMT_ATTR_FALLTHROUGH();
            case HTTP_STATE_REQUEST_METHOD:
                YMO_STMT_ATTR_FALLTHROUGH();
            case HTTP_STATE_REQUEST_URI_PATH:
                YMO_STMT_ATTR_FALLTHROUGH();
            case HTTP_STATE_REQUEST_VERSION:
                n = ymo_parse_http_request_line(
                        exchange, parse_buf, len);
                break;
            case HTTP_STATE_CRLF:
                n = ymo_parse_http_crlf(
                        exchange, parse_buf, len);
                break;
            case HTTP_STATE_HEADER_NAME:
                YMO_STMT_ATTR_FALLTHROUGH();
            case HTTP_STATE_HEADER_VALUE_LEADING_SPACE:
                YMO_STMT_ATTR_FALLTHROUGH();
            case HTTP_STATE_HEADER_VALUE:
                n = ymo_parse_http_headers(
                        http_proto_data->header_cb,
                        http_session,
                        exchange,
                        parse_buf,
                        len);
                break;
            case HTTP_STATE_EXPECT:
                goto send_100_continue;
                break;
            case HTTP_STATE_BODY_CHUNK_HEADER:
                YMO_STMT_ATTR_FALLTHROUGH();
            case HTTP_STATE_BODY:
                YMO_STMT_ATTR_FALLTHROUGH();
            case HTTP_STATE_BODY_CHUNK_TRAILER:
                n = ymo_parse_http_body(
                        http_proto_data->body_cb,
                        http_session,
                        exchange,
                        parse_buf,
                        len);
                break;
            case HTTP_STATE_COMPLETE:
                goto http_parse_complete;
                break;
            default:
                break;
        }
        if( n >= 0 ) {
            HTTP_PROTO_TRACE(
                    "%zi bytes parsed:\n"
                    "State: %s (next: %s); remain: %zu",
                    n,
                    HTTP_PARSE_STATE_NAME(exchange->state),
                    HTTP_PARSE_STATE_NAME(exchange->next_state),
                    len);
            parse_buf += n;
            len -= n;
        } else {
            /* Errno has been set by last parse function. */
            return -1;
        }

    } while( len );

    if( exchange->state != HTTP_STATE_EXPECT ) {
        /* If the exchange is complete, invoke the http exchange handler: */
        if( exchange->state == HTTP_STATE_COMPLETE ) {
http_parse_complete:
            {
                HTTP_PROTO_TRACE("HTTP exchange complete:\n"
                        "\tversion: \"%s\"\n"
                        "\tmethod: \"%s\"\n"
                        "\turi: \"%s\"\n"
                        "\tquery: \"%s\"\n"
                        "\tfragment: \"%s\"\n"
                        "\tcontent-length: %zu\n",
                        exchange->request.version,
                        exchange->request.method,
                        exchange->request.uri,
                        exchange->request.query,
                        exchange->request.fragment,
                        exchange->request.content_length);
                ymo_status_t status = ymo_http_handler(
                        conn->server, conn, http_proto_data,
                        http_session, exchange);
                if( status != YMO_OKAY && !YMO_IS_BLOCKED(status) ) {
                    errno = status;
                    return -1;
                }

                if( ymo_server_get_state(conn->server) == YMO_SERVER_STOP_GRACEFUL ) {
                    /* if the server's shutting down and we just handled a exchange,
                     * stop reading from this connection:
                     */
                    ymo_conn_rx_enable(conn, 0);
                }
            }
        }
    } else {
send_100_continue:
        {
            /* If so, issue an HTTP 100 right now (fingers crossed we don't
             * block on send...).
             */
            errno = 0;
            ymo_http_response_t* res_expect
                = ymo_http_response_create(http_session);
            HTTP_PROTO_TRACE("Setting expect response to 100: %p",
                    (void*)res_expect);
            if( res_expect ) {
                /* HACK: set the keep-alive flag so that 100 response is sent
                 * without closing on HTTP/1.0.
                 */
                res_expect->flags = YMO_HTTP_FLAG_REQUEST_KEEPALIVE;
                ymo_http_session_add_response(http_session, res_expect);
                status = ymo_http_response_issue(res_expect, 100);
                ymo_conn_tx_now(conn);
            } else {
                ymo_log_error("Failed to create Expect response: %s",
                        strerror(errno));
                errno = ENOMEM;
                return -1;
            }

            /* Untoggle expect so we don't come back here: */
            exchange->request.flags &= ~(YMO_HTTP_FLAG_EXPECT);
            if( exchange->request.flags & YMO_HTTP_REQUEST_CHUNKED ) {
                exchange->state = HTTP_STATE_BODY_CHUNK_HEADER;
            } else {
                exchange->state = HTTP_STATE_BODY;
            }
            exchange->next_state = HTTP_STATE_COMPLETE;
        }
    }
    return (parse_buf - recv_buf);
}


/*---------------------------------------------------------------*
 *  Yimmo HTTP Protocol Write Callback:
 *---------------------------------------------------------------*/
ymo_status_t ymo_proto_http_write(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        int socket)
{
    static const char* chunk_term = "0\r\n\r\n";
    ymo_status_t status;
    ymo_http_session_t* http_session = conn_data;
    ymo_http_response_t* response = ymo_http_session_next_response(http_session);
#if 0
    HTTP_PROTO_TRACE("%i: YMO_HTTP_FLAG_SUPPORTS_CHUNKED: 0x%x", socket, response->flags & YMO_HTTP_FLAG_SUPPORTS_CHUNKED);
    HTTP_PROTO_TRACE("%i: YMO_HTTP_RESPONSE_CHUNKED: 0x%x", socket, response->flags & YMO_HTTP_RESPONSE_CHUNKED);
    HTTP_PROTO_TRACE("%i: YMO_HTTP_RESPONSE_READY: 0x%x", socket, response->flags & YMO_HTTP_RESPONSE_READY);
    HTTP_PROTO_TRACE("%i: YMO_HTTP_RESPONSE_STARTED: 0x%x", socket, response->flags & YMO_HTTP_RESPONSE_STARTED);
    HTTP_PROTO_TRACE("%i: YMO_HTTP_RESPONSE_COMPLETE: 0x%x", socket, response->flags & YMO_HTTP_RESPONSE_COMPLETE);
#endif

    if( !response ) {
        ymo_log_debug("Got writable callback, but nothing to send on %i",
                socket);
        return YMO_OKAY;
    }

    ymo_http_flags_t r_flags = response->flags;

    if( !(r_flags & YMO_HTTP_RESPONSE_READY) ) {
        ymo_log_debug("Got writable callback, but response not ready on %i",
                socket);
        return YMO_OKAY;
    }

    /* TODO: fold this into the clause above or below. */
    if( !(r_flags & (YMO_HTTP_FLAG_SUPPORTS_CHUNKED|YMO_HTTP_RESPONSE_COMPLETE)) ) {
        HTTP_PROTO_TRACE("Have body data, but connection doesn't support transfer-encoding-chunked on %i. Response will be buffered.", socket);
        return YMO_OKAY;
    }

    if( !(r_flags & YMO_HTTP_RESPONSE_STARTED) ) {
        errno = 0;
        http_session->send_buffer = ymo_http_response_start(conn, response);
        if( !http_session->send_buffer ) {
            ymo_log_debug("Error serializing response: %s", strerror(errno));
            return errno;
        }

        HTTP_PROTO_TRACE("Response started on %i", socket);
        response->flags |= YMO_HTTP_RESPONSE_STARTED;
    }

    errno = 0;
    ymo_bucket_t* body_data = ymo_http_response_body_get(conn, response);
    if( body_data ) {
        if( http_session->send_buffer ) {
            ymo_bucket_append(http_session->send_buffer, body_data);
        } else {
            http_session->send_buffer = body_data;
        }
    } else if( errno ) {
        ymo_log_debug("Error retrieving body data: %s", strerror(errno));
        return errno;
    }

    /* HACK: add terminal chunk: */
    if( (response->flags & YMO_HTTP_RESPONSE_COMPLETE)
        && (response->flags & YMO_HTTP_RESPONSE_CHUNKED)
        && !(response->flags & YMO_HTTP_RESPONSE_CHUNK_TERM) ) {
        HTTP_PROTO_TRACE("Appending terminal chunk for %i", socket);
        body_data = YMO_BUCKET_FROM_REF(chunk_term, 5);
        if( http_session->send_buffer ) {
            ymo_bucket_append(http_session->send_buffer, body_data);
        } else {
            http_session->send_buffer = body_data;
        }

        response->flags |= YMO_HTTP_RESPONSE_CHUNK_TERM;
    }

#if 0
    if( http_session->send_buffer ) {
#endif
    /* We're good to write: */
    status = ymo_conn_send_buckets(
            conn, &http_session->send_buffer);
#if 0
} else {
    /* We're either done with this response, or else waiting on more
     * data.
     * We set status to YMO_OKAY, as if we had send something.
     * If the response is complete, we'll wrap it up, and check for
     * another one in the queue:
     *   - next response ready?: YMO_WOULDBLOCK (leave tx enabled)
     *   - no responsee ready?: OKAY (tx disable)
     *
     * If the response is NOT complete, we'll return OKAY (tx disable)
     * and have faith that someone will invoke response_finish to toggle
     * the flag and bring us back here on another loop iteration in the
     * future.
     */
    status = YMO_OKAY;
}
#endif

    /* This response has been fully sent: */
    if( status == YMO_OKAY && (r_flags & YMO_HTTP_RESPONSE_COMPLETE) ) {
        http_session->send_buffer = NULL;
        ymo_http_flags_t response_flags = response->flags;
        ymo_http_session_complete_response(http_session);

        /* If this was an upgrade response, we can transition protocols now. */
        if( response->proto_new ) {
            return ymo_conn_transition_proto(conn, response->proto_new);
        }

        /* Close after response complete if keep-alive not set */
        if( !(response_flags & YMO_HTTP_FLAG_REQUEST_KEEPALIVE) ) {
            HTTP_PROTO_TRACE("Keep-alive flag not set (%i & %i = %i). Closing",
                    response_flags, YMO_HTTP_FLAG_REQUEST_KEEPALIVE,
                    response_flags & YMO_HTTP_FLAG_REQUEST_KEEPALIVE);
            status = EPIPE; /* TODO: this is weird */
        }

        /* If there are more ready responses, return YMO_WOULDBLOCK to keep the
         * write watcher enabled: */
        ymo_http_response_t* next =
            ymo_http_session_next_response(http_session);
        if( next && next->flags & YMO_HTTP_RESPONSE_READY ) {
            status = YMO_WOULDBLOCK;
        }
    }

    return status;
}



