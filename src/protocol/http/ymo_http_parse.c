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

/* HACK HACK: for strdup */
#define _POSIX_C_SOURCE 200809L
#include "ymo_config.h"

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_util.h"
#include "ymo_http_hdr_table.h"
#include "ymo_http_parse.h"
#include "ymo_http_exchange.h"
#include "ymo_http_response.h"
#include "ymo_http_session.h"

/* From RFC 7230: header field-names are tokens.
 *
 * tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
 *         "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
 * token = 1*tchar
 * field-name = token
 *
 *
 * (https://datatracker.ietf.org/doc/html/rfc7230#appendix-B)
 */

/*---------------------------------------------------------------*
 *  Declarations
 *---------------------------------------------------------------*/
#define STD_HTTP_VERSION_STR_LEN 8

#define YMO_HTTP_TRACE_PARSE 0
#if defined(YMO_HTTP_TRACE_PARSE) && YMO_HTTP_TRACE_PARSE == 1
#define HTTP_PARSE_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__);

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
#define HTTP_PARSE_TRACE(fmt, ...)
#define HTTP_PARSE_STATE_NAME(x)
#endif /* YMO_HTTP_TRACE_PARSE */

/*---------------------------------------------------------------*
 *  Static Functions:
 *---------------------------------------------------------------*/
static inline ssize_t parser_saw_cr(
        ymo_http_exchange_t* exchange,
        http_state_t next_state) YMO_FUNC_FLATTEN;


/** Used by parsing functions to mark CR and set up parser to
 * handle HTTP CRLF end of line sequence appropriately. */
static inline ssize_t parser_saw_cr(
        ymo_http_exchange_t* exchange, http_state_t next_state)
{
    HTTP_PARSE_TRACE("Saw carriage return (%p)",
            (void*)exchange);
    exchange->state = HTTP_STATE_CRLF;
    exchange->next_state = next_state;
    return 1;
}


/** Called by ymo_parse_request when the exchange line has been parsed.
 *
 * - validate method
 * - validate HTTP version
 * - determine the next parse/handler action
 *
 * TODO: CHECK for valid ASCII/URI chars!
 */
static ymo_status_t check_request(
        http_state_t* next_state,
        ymo_http_exchange_t* exchange,
        size_t version_len)
{
    HTTP_PARSE_TRACE("Analyzing HTTP exchange (%p) for \"%s\"",
            (void*)exchange,
            exchange->request.uri);

#define HACK_HTTP_V(c1, c2) ((c1<<8) | c2)
#define HACK_HTTP_10 (('1'<<8) | '0')
#define HACK_HTTP_11 (('1'<<8) | '1')
    /* HACK: hard-code for standard version strings: */
    if( version_len == STD_HTTP_VERSION_STR_LEN ) {
        uint16_t http_ver = HACK_HTTP_V(
                exchange->request.version[5],
                exchange->request.version[7]
                );

        switch( http_ver ) {
            case HACK_HTTP_11:
                exchange->request.flags \
                    |= (YMO_HTTP_FLAG_VERSION_1_1
                        | YMO_HTTP_FLAG_SUPPORTS_CHUNKED
                        | YMO_HTTP_FLAG_REQUEST_KEEPALIVE);
            case HACK_HTTP_10:
                break;
            default:
                goto bail_malformed_http_status;
                break;
        }
    } else {
        goto bail_malformed_http_status;
    }

    *next_state = HTTP_STATE_HEADER_NAME;
    return YMO_OKAY;

bail_malformed_http_status:
    /* TODO: HTTP 505 right here */
    ymo_log_debug("Malformed HTTP version string: \"%s\" (%zu)",
            exchange->request.version, version_len);
    return EINVAL;
}


/* TODO: move this to another translation unit. */
static ymo_status_t ymo_http_session_init_response(
        ymo_http_session_t* session,
        ymo_http_exchange_t* exchange)
{
    ymo_status_t status = YMO_OKAY;

    /* Else, start prepping the app-facing response: */
    ymo_http_response_t* response = ymo_http_response_create(session);

    if( !response ) {
        ymo_log_error("Unable to create HTTP response object: %s",
                strerror(errno));
        status = ENOMEM;
    }

    exchange->response = response;
    response->flags = exchange->request.flags;
    if( exchange->request.flags & YMO_HTTP_FLAG_REQUEST_KEEPALIVE ) {
        if( !(exchange->request.flags & YMO_HTTP_FLAG_VERSION_1_1) ) {
#define USE_PRECOMPUTE
#ifdef USE_PRECOMPUTE
            ymo_http_hdr_table_insert_precompute(
                    &response->headers, HDR_ID_CONNECTION,
                    "Connection", sizeof("Connection")-1, "Keep-alive");
#else
            ymo_http_hdr_table_insert(
                    &response->headers, "Connection", "Keep-alive");
#endif
        }
    } else {
        if( exchange->request.flags & YMO_HTTP_FLAG_VERSION_1_1 ) {
            ymo_http_hdr_table_insert_precompute(
                    &response->headers, HDR_ID_CONNECTION,
                    "Connection", sizeof("Connection")-1, "close");
        }
    }

    return status;
}


/** Called by ymo_parse_http_headers when the header terminal is encountered.
 *
 * Performs the following actions after all headers have been received:
 * - validate header values
 * - check for available buffer space, if a body is being sent unchunked
 * - determine the next parse/handler action
 *
 * TODO: provide user callback (expect handling can be shuffled there too).
 */
static ymo_status_t check_headers(
        ymo_http_session_t* session,
        ymo_http_exchange_t* exchange,
        const char** current)
{
    HTTP_PARSE_TRACE("Analyzing HTTP headers (%p) for \"%s\"",
            (void*)exchange,
            exchange->request.uri);
    http_state_t next_state = HTTP_STATE_COMPLETE;

    if( exchange->request.content_length > 0 ) {
        next_state = HTTP_STATE_BODY;
        exchange->body_remain = exchange->request.content_length;

        /* Adjust the state if the client was audacious enough to send
         * Expect: 100-continue:
         */
        if( exchange->request.flags & YMO_HTTP_FLAG_EXPECT ) {
            next_state = HTTP_STATE_EXPECT;
        }

    } else if( exchange->request.flags & YMO_HTTP_REQUEST_CHUNKED ) {
        exchange->chunk_current = exchange->chunk_hdr;
        next_state = HTTP_STATE_BODY_CHUNK_HEADER;
    }

    /* Adjust the state if the client was audacious enough to send
     * Expect: 100-continue.
     *
     * TODO: move this to user code now that we have header_cb?
     */
    if( exchange->request.flags & YMO_HTTP_FLAG_EXPECT ) {
        next_state = HTTP_STATE_EXPECT;
    }

    ymo_http_session_init_response(session, exchange);
    *current += parser_saw_cr(exchange, next_state);
    HTTP_PARSE_TRACE(
            "jumping to state (%p): %s", (void*)exchange,
            HTTP_PARSE_STATE_NAME(next_state));
    return YMO_OKAY;
}


/*---------------------------------------------------------------*
 *  Yimmo HTTP Parse Functions:
 *---------------------------------------------------------------*/
/* HTTP exchange line parser: */
ssize_t ymo_parse_http_request_line(
        ymo_http_exchange_t* exchange, const char* buffer, size_t len)
{
    register char c;
    size_t remain = exchange->remain;
    char* recv_current = exchange->recv_current;
    const char* current = buffer;
    const char* buf_end = buffer + len;
    HTTP_PARSE_TRACE("%p parsing exchange line; state: %s",
            (void*)exchange, HTTP_PARSE_STATE_NAME(exchange->state));

    /* NOTE: EXP_STATUS_PARSER makes about *zero* difference.... */
    http_state_t state = exchange->state;
    const char** up_ptr = NULL;
    do {
        c = *current;

        switch( state ) {
            case HTTP_STATE_REQUEST_METHOD:
                if( c != ' ' ) {
                    break;
                } else {
                    up_ptr = &exchange->request.uri;
                    goto status_item_next;
                }
                break;
            case HTTP_STATE_REQUEST_URI_PATH:
                switch( c ) {
                    case '?':
                        up_ptr = &exchange->request.query;
                        goto status_item_next;
                        break;
                    case '#':
                        /* HACK: if the path ended without '?', jump to
                         * fragment. */
                        exchange->request.query = NULL;
                        exchange->state++;
                        up_ptr = &exchange->request.fragment;
                        goto status_item_next;
                        break;
                    case ' ':
                        /* HACK: if the path ended without '?' or '#'
                         * then skip query + fragment:
                         */
                        exchange->request.query = exchange->request.fragment = NULL;
                        exchange->state += 2;
                        up_ptr = &exchange->request.version;
                        goto status_item_next;
                    default:
                        goto status_item_continue;
                        break;
                }
                break;
            case HTTP_STATE_REQUEST_QUERY:
                switch( c ) {
                    case '#':
                        up_ptr = &exchange->request.fragment;
                        goto status_item_next;
                        break;
                    case ' ':
                        /* HACK: if the query ended without '#',
                         * then skip the fragment:
                         */
                        exchange->request.fragment = NULL;
                        exchange->state++;
                        up_ptr = &exchange->request.version;
                        goto status_item_next;
                        break;
                    default:
                        break;
                }
                break;
            case HTTP_STATE_REQUEST_FRAGMENT:
                if( c != ' ' ) {
                    break;
                } else {
                    up_ptr = &exchange->request.version;
                    goto status_item_next;
                }
                break;
            case HTTP_STATE_REQUEST_VERSION:
                if( c != '\r' ) {
                    break;
                } else {
                    http_state_t next_state;
                    size_t v_len =
                        (recv_current - exchange->request.version);
                    ymo_status_t line_status = \
                        check_request(&next_state, exchange, v_len);
                    if( line_status == YMO_OKAY ) {
                        *(recv_current++) = '\0';
                        current += parser_saw_cr(
                                exchange, HTTP_STATE_HEADER_NAME);
                        goto http_request_parse_done;
                    } else {
                        exchange->recv_current = recv_current;
                        return YMO_ERROR_SSIZE_T(line_status);
                    }
                }
                break;
            default:
                goto http_request_parse_done;
                break;
        }

status_item_continue:
        *(recv_current++) = c;
        --remain;
        continue;

status_item_next:
        *(recv_current++) = '\0';
        *up_ptr = recv_current;
        state = ++exchange->state;
        --remain;
    } while( ++current < buf_end && remain );


http_request_parse_done:
    if( remain ) {
        exchange->recv_current = recv_current;
        exchange->remain = remain;
        return (current - buffer);
    }

    return YMO_ERROR_SSIZE_T(EFBIG);
}


/* HTTP end-of-line parser: */
ssize_t ymo_parse_http_crlf(
        ymo_http_exchange_t* exchange, const char* buffer, size_t len)
{
    char c = *buffer;
    HTTP_PARSE_TRACE("parsing CRLF (%p)", (void*)exchange);

    if( c != '\n' ) {
        goto bail_malformed_crlf;
    }

    /* Jump to next HTTP state, as set in parser_saw_cr: */
    exchange->state = exchange->next_state;
    switch( exchange->state ) {
        case HTTP_STATE_HEADER_NAME:
            exchange->hdr_name = \
                exchange->recv_current;
            break;
        case HTTP_STATE_EXPECT:
            break;
        case HTTP_STATE_BODY_CHUNK_HEADER:
            exchange->next_state = HTTP_STATE_BODY_CHUNK_TRAILER;
            break;
        case HTTP_STATE_BODY:
            exchange->next_state = HTTP_STATE_COMPLETE;
            break;
        case HTTP_STATE_COMPLETE:
            break;
        default:
            break;
    }
    HTTP_PARSE_TRACE("got LF (%p); jump to state: %s",
            (void*)exchange, HTTP_PARSE_STATE_NAME(exchange->state));
    goto http_crlf_parse_done;

http_crlf_parse_done:
    return 1;

bail_malformed_crlf:
    ymo_log_debug("Malformed HTTP exchange: bad CRLF; got 0x%x", (int)c);
    /* TODO: Issue HTTP 400 */
    return YMO_ERROR_SSIZE_T(EBADMSG);
}


/* HTTP header parser: */
ssize_t ymo_parse_http_headers(
        ymo_http_header_cb_t header_cb,
        ymo_http_session_t* session,
        ymo_http_exchange_t* exchange,
        const char* buffer, size_t len)
{
    char c;
    size_t remain = exchange->remain;
    char* recv_current = exchange->recv_current;
    const char* current = buffer;
    const char* buf_end = buffer + len;
    HTTP_PARSE_TRACE("parsing headers (%p); state: %s",
            (void*)exchange, HTTP_PARSE_STATE_NAME(exchange->state));
    do {
        c = *current;
        switch( exchange->state ) {
            case HTTP_STATE_HEADER_NAME:
                /* Most of the time it's not ':' */
                if( c != ':' ) {
                    /* When it's not ':', most of the time it's not '\r'. */
                    if( c != '\r' ) {
                        exchange->h_id = YMO_HDR_HASH_CH(exchange->h_id, c);
                        break;
                    } else {
                        /* If we see CR, we're done with headers.*/
                        ymo_status_t header_status =
                            check_headers(session, exchange, &current);
                        if( header_status == YMO_OKAY ) {
                            if( header_cb
                                /* HACK: don't issue header cb for
                                 *       upgrade requests (for now): */
                                && !(exchange->request.flags & YMO_HTTP_FLAG_UPGRADE)
                                ) {
                                ymo_status_t cb_status = header_cb(
                                        session,
                                        &exchange->request,
                                        exchange->response,
                                        session->user_data);
                                if( cb_status != YMO_OKAY ) {
                                    return YMO_ERROR_SSIZE_T(cb_status);
                                }
                            }
                            goto http_header_parse_done;
                        } else {
                            /* Something is wrong with the exchange!
                             * Set errno and bail: */
                            exchange->recv_current = recv_current;
                            return YMO_ERROR_SSIZE_T(header_status);
                        }
                    }
                } else {
                    *(recv_current++) = '\0';
                    --remain;
                    HTTP_PARSE_TRACE("got header (%p): %s",
                            (void*)exchange, exchange->hdr_name);
                    exchange->state = \
                        HTTP_STATE_HEADER_VALUE_LEADING_SPACE;
                    continue;
                }
                break;
            case HTTP_STATE_HEADER_VALUE_LEADING_SPACE:
                /* Swallowing leading space between name and value: */
                if( c == ' ' ) {
                    continue;
                }
                exchange->hdr_value = recv_current;
                exchange->state = HTTP_STATE_HEADER_VALUE;
                YMO_STMT_ATTR_FALLTHROUGH();
            case HTTP_STATE_HEADER_VALUE:
                if( c != '\r' ) {
                    break;
                } else {
                    *(recv_current++) = '\0';
                    --remain;
                    HTTP_PARSE_TRACE("got \"%s\" value (%p): \"%s\"",
                            exchange->hdr_name,
                            (void*)exchange,
                            exchange->hdr_value);

                    exchange->h_id = exchange->h_id & YMO_HDR_TABLE_MASK;
                    ymo_http_hdr_table_add_precompute(
                            &exchange->request.headers,
                            exchange->h_id,
                            exchange->hdr_name,
                            (exchange->hdr_value - exchange->hdr_name)-1,
                            exchange->hdr_value);

                    switch( exchange->h_id ) {
                        case HDR_ID_CONNECTION:
                            /* 1.1: default to keep-alive unless "close": */
                            if( exchange->request.flags & YMO_HTTP_FLAG_VERSION_1_1 ) {
                                /* TODO: we know the lengths of these. Use
                                 * a ymo_strncmp function which checks length
                                 * first?
                                 *
                                 * Eh... the input isn't sorted or likely to
                                 * be super close, so... what's the benefit?
                                 */
                                if( !strcasecmp(exchange->hdr_value, "close") ) {
                                    exchange->request.flags &= \
                                        YMO_HTTP_FLAG_REQUEST_CLOSE;
                                }
                            } else {
                                /* 1.0: default to close unless "keep-alive":
                                 *
                                 * TODO: what about "keep-alive, Upgrade"?
                                 *       strcasestr?
                                 */
                                if( !strcasecmp(exchange->hdr_value, "keep-alive") ) {
                                    exchange->request.flags |= \
                                        YMO_HTTP_FLAG_REQUEST_KEEPALIVE;
                                }
                            }
                            break;
                        case HDR_ID_CONTENT_LENGTH:
                        {
                            long content_length;
                            char* end_ptr;
                            content_length =
                                strtol(exchange->hdr_value, &end_ptr, 10);
                            if( *end_ptr == '\0' ) {
                                exchange->request.content_length = \
                                    (size_t)content_length;

                                exchange->request.content_length =
                                    content_length;
                            }
                        }
                        break;
                        case HDR_ID_EXPECT:
                            if( !strcasecmp(exchange->hdr_value, "100-continue") ) {
                                exchange->request.flags |= YMO_HTTP_FLAG_EXPECT;
                            }
                            break;
                        case HDR_ID_UPGRADE:
                            exchange->request.flags |= YMO_HTTP_FLAG_UPGRADE;
                            break;
                        case HDR_ID_TRANSFER_ENCODING:
                            if( !strcasecmp(exchange->hdr_value, "chunked")) {
                                exchange->request.flags |= YMO_HTTP_REQUEST_CHUNKED;
                            }
                            break;
                        default:
                            break;
                    }
                    exchange->h_id = YMO_HTTP_HDR_HASH_INIT();
                    current += parser_saw_cr(
                            exchange, HTTP_STATE_HEADER_NAME);
                    goto http_header_parse_done;
                }
                break;
            default:
                goto http_header_parse_done;
                break;
        }

        /* Since we're here, store the char we just saw. */
        *(recv_current++) = c;
        --remain;
    } while( ++current < buf_end && remain );

http_header_parse_done:
    if( remain ) {
        exchange->recv_current = recv_current;
        exchange->remain = remain;
        return (current - buffer);
    }

    return YMO_ERROR_SSIZE_T(EFBIG);
}


/** HTTP body parser
 * TODO: store in buckets!
 */
ssize_t ymo_parse_http_body(
        ymo_http_body_cb_t body_cb,
        ymo_http_session_t* session,
        ymo_http_exchange_t* exchange,
        const char* buffer,
        size_t len)
{
    const char* current = buffer;

    do {
        switch( exchange->state ) {
            case HTTP_STATE_BODY_CHUNK_HEADER:
                do {
                    switch( *current ) {
                        case '\r':
                            ++current;
                            --len;
                            *exchange->chunk_current++ = '\0';
                            break;
                        case '\n':
                            ++current;
                            --len;
                            exchange->body_remain =
                                (size_t)strtol(exchange->chunk_hdr, NULL, 16);

                            HTTP_PARSE_TRACE("Got chunk header: %s (%zu)",
                                    exchange->chunk_hdr, exchange->body_remain);
                            exchange->chunk_current = exchange->chunk_hdr;

                            if( exchange->body_remain ) {
                                exchange->state = HTTP_STATE_BODY;
                                exchange->next_state = HTTP_STATE_BODY_CHUNK_TRAILER;
                            } else {
                                exchange->state = HTTP_STATE_BODY_CHUNK_TRAILER;
                                exchange->next_state = HTTP_STATE_COMPLETE;
                            }
                            HTTP_PARSE_TRACE(
                                    "Jumping to state: %s",
                                    state_names[exchange->state]);
                            goto body_hdr_done;
                            break;
                        default:
                            *exchange->chunk_current++ = *current;
                            ++current;
                            --len;
                            break;
                    }
                } while( len );
body_hdr_done:
                break;
            case HTTP_STATE_BODY:
            {
                if( !exchange->body_remain ) {
                    HTTP_PARSE_TRACE("Next state: %s", "COMPLETE");
                    exchange->state = HTTP_STATE_COMPLETE;
                    break;
                }

                size_t body_available = YMO_MIN(len, exchange->body_remain);

                ymo_status_t cb_status = body_cb(
                        session,
                        &exchange->request,
                        exchange->response,
                        current,
                        body_available,
                        session->user_data);

                if( cb_status == YMO_OKAY ) {
                    len -= body_available;
                    exchange->body_remain -= body_available;
                    current += body_available;

                    if( !exchange->body_remain ) {
                        if( exchange->request.flags & YMO_HTTP_REQUEST_CHUNKED ) {
                            exchange->state = HTTP_STATE_BODY_CHUNK_TRAILER;
                            exchange->next_state = HTTP_STATE_BODY_CHUNK_HEADER;
                        } else {
                            exchange->state = exchange->next_state;
                        }
                        HTTP_PARSE_TRACE(
                                "Jumping to state: %s",
                                state_names[exchange->state]);
                    }
                } else {
                    return YMO_ERROR_SSIZE_T(cb_status);
                }
            }
            break;
            case HTTP_STATE_BODY_CHUNK_TRAILER:
                do {
                    /* HACK: just loop until newline (this is sloppy...) */
                    if( *current++ == '\n' ) {
                        exchange->state = exchange->next_state;
                        HTTP_PARSE_TRACE(
                                "Jumping to state: %s",
                                state_names[exchange->state]);
                        break;
                    }
                } while( --len );
                break;
            default:
                goto body_parse_done;
                break;
        }
    } while( len );

body_parse_done:
    return (current-buffer);
}

