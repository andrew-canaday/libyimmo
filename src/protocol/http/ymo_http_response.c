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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_alloc.h"
#include "core/ymo_bucket.h"
#include "core/ymo_server.h"
#include "core/ymo_conn.h"
#include "ymo_http_response.h"

static const char* get_http_reason(ymo_http_status_t status)
{
    const char* reason = NULL;
    switch( status ) {
        case YMO_HTTP_CONTINUE:
            reason = "Continue";
            break;
        case YMO_HTTP_SWITCHING_PROTOCOLS:
            reason = "Switching Protocols";
            break;
        case YMO_HTTP_OK:
            reason = "OK";
            break;
        case YMO_HTTP_CREATED:
            reason = "Created";
            break;
        case YMO_HTTP_ACCEPTED:
            reason = "Accepted";
            break;
        case YMO_HTTP_NON_AUTHORITATIVE_INFORMATION:
            reason = "Non-Authoritative Information";
            break;
        case YMO_HTTP_NO_CONTENT:
            reason = "No Content";
            break;
        case YMO_HTTP_RESET_CONTENT:
            reason = "Reset Content";
            break;
        case YMO_HTTP_PARTIAL_CONTENT:
            reason = "Partial Content";
            break;
        case YMO_HTTP_MULTIPLE_CHOICES:
            reason = "Multiple Choices";
            break;
        case YMO_HTTP_MOVED_PERMANENTLY:
            reason = "Moved Permanently";
            break;
        case YMO_HTTP_FOUND:
            reason = "Found";
            break;
        case YMO_HTTP_SEE_OTHER:
            reason = "See Other";
            break;
        case YMO_HTTP_NOT_MODIFIED:
            reason = "Not Modified";
            break;
        case YMO_HTTP_USE_PROXY:
            reason = "Use Proxy";
            break;
        case YMO_HTTP_TEMPORARY_REDIRECT:
            reason = "Temporary Redirect";
            break;
        case YMO_HTTP_BAD_REQUEST:
            reason = "Bad Request";
            break;
        case YMO_HTTP_UNAUTHORIZED:
            reason = "Unauthorized";
            break;
        case YMO_HTTP_PAYMENT_REQUIRED:
            reason = "Payment Required";
            break;
        case YMO_HTTP_FORBIDDEN:
            reason = "Forbidden";
            break;
        case YMO_HTTP_NOT_FOUND:
            reason = "Not Found";
            break;
        case YMO_HTTP_METHOD_NOT_ALLOWED:
            reason = "Method Not Allowed";
            break;
        case YMO_HTTP_NOT_ACCEPTABLE:
            reason = "Not Acceptable";
            break;
        case YMO_HTTP_PROXY_AUTHENTICATION_REQUIRED:
            reason = "Proxy Authentication Required";
            break;
        case YMO_HTTP_REQUEST_TIMEOUT:
            reason = "Request Timeout";
            break;
        case YMO_HTTP_CONFLICT:
            reason = "Conflict";
            break;
        case YMO_HTTP_GONE:
            reason = "Gone";
            break;
        case YMO_HTTP_LENGTH_REQUIRED:
            reason = "Length Required";
            break;
        case YMO_HTTP_PRECONDITION_FAILED:
            reason = "Precondition Failed";
            break;
        case YMO_HTTP_REQUEST_ENTITY_TOO_LARGE:
            reason = "Request Entity Too Large";
            break;
        case YMO_HTTP_REQUEST_URI_TOO_LONG:
            reason = "Request URI Too long";
            break;
        case YMO_HTTP_UNSUPPORTED_MEDIA_TYPE:
            reason = "Unsupported Media Type";
            break;
        case YMO_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE:
            reason = "Requested Range Not Satisfiable";
            break;
        case YMO_HTTP_EXPECTATION_FAILED:
            reason = "Expectation Failed";
            break;
        case YMO_HTTP_INTERNAL_SERVER_ERROR:
            reason = "Internal Server Error";
            break;
        case YMO_HTTP_NOT_IMPLEMENTED:
            reason = "Not Implemented";
            break;
        case YMO_HTTP_BAD_GATEWAY:
            reason = "Bad Gateway";
            break;
        case YMO_HTTP_SERVICE_UNAVAILABLE:
            reason = "Service Unavailable";
            break;
        case YMO_HTTP_GATEWAY_TIMEOUT:
            reason = "Gateway Timeout";
            break;
        case YMO_HTTP_HTTP_VERSION_NOT_SUPPORTED:
            reason = "HTTP Version Not Supported";
            break;
        default:
            break;
    }
    return reason;
}


ymo_http_response_t* ymo_http_response_create(ymo_http_session_t* session)
{
    ymo_http_response_t* response = NULL;
    response = YMO_NEW0(ymo_http_response_t);
    if( response ) {
        response->session = session;
        response->status = 200;

        ymo_http_hdr_table_init(&response->headers);
    }
    return response;
}


ymo_http_hdr_table_t* ymo_http_response_get_headers(
        ymo_http_response_t* response)
{
    return &response->headers;
}


const char* ymo_http_response_get_header(
        const ymo_http_response_t* response, const char* hdr_name)
{
    return ymo_http_hdr_table_get(&response->headers, hdr_name);
}


ymo_status_t ymo_http_response_add_header(
        ymo_http_response_t* response, const char* header, const char* value)
{
    ymo_http_hdr_table_insert(&response->headers, header, value);
    return YMO_OKAY;
}


ymo_status_t ymo_http_response_insert_header(
        ymo_http_response_t* response, const char* header, const char* value)
{
    ymo_http_hdr_table_insert(&response->headers, header, value);
    return YMO_OKAY;
}


void ymo_http_response_body_append(
        ymo_http_response_t* response, ymo_bucket_t* body_data)
{
    if( !response->body_head ) {
        response->body_head = response->body_tail = body_data;
    } else {
        response->body_tail = ymo_bucket_append(response->body_tail, body_data);
    }

    /* Only enable writes and set the response as ready if chunked encoding
     * is available on this response (else, we wait until we've got the whole
     * thing in _finish().
     */
    if( response->flags & YMO_HTTP_FLAG_SUPPORTS_CHUNKED ) {
        response->flags |= YMO_HTTP_RESPONSE_READY;
        ymo_conn_tx_enable(response->session->conn, 1);
    }
    return;
}


void ymo_http_response_set_status(
        ymo_http_response_t* response, ymo_http_status_t status)
{
    response->status = status;
    response->status_len = snprintf(
            response->status_str, STATUS_STR_MAX_LEN, "%i %s",
            response->status, get_http_reason(response->status));
}


#define TRUST_STATUS_STR 1

int ymo_http_response_set_status_str(
        ymo_http_response_t* response, const char* status_str)
{
    char* write_end = memccpy(
            response->status_str, status_str, '\0', STATUS_STR_MAX_LEN);
    response->status_len = (write_end - response->status_str)-1;
#if defined(TRUST_STATUS_STR) && (TRUST_STATUS_STR == 1)
    response->status = YMO_HTTP_STATUS_CHR3_TO_INT(status_str);
#else
    char* end_ptr;
    response->status = (ymo_http_status_t)strtol(status_str, &end_ptr, 10);
    if( end_ptr == NULL || response->status > 599 ) {
        return 0;
    }
#endif /* TRUST_STATUS_STR */

    return 1;
}


ymo_status_t ymo_http_response_issue(
        ymo_http_response_t* response, ymo_http_status_t status)
{
    if( status >= 400 ) {
        ymo_http_response_insert_header(response, "Connection", "Close");
        /* TODO: not sure about response having session... */
        response->session->state = YMO_HTTP_SESSION_ERROR;
    }
    ymo_http_response_set_status(response, status);
    ymo_http_response_finish(response);
    return YMO_OKAY;
}


ymo_http_flags_t ymo_http_response_flags(const ymo_http_response_t* response)
{
    return response->flags;
}


/* TODO: should probably have a set/unset, take a param, or just expose the
 * flags...
 */
void ymo_http_response_set_flag(
        ymo_http_response_t* response, ymo_http_flags_t flag)
{
    response->flags |= flag;
}


void ymo_http_response_set_flags(
        ymo_http_response_t* response, ymo_http_flags_t flags)
{
    response->flags = flags;
}


void ymo_http_response_ready(ymo_http_response_t* response)
{
    response->flags |= YMO_HTTP_RESPONSE_READY;
    ymo_conn_tx_enable(response->session->conn, 1);
    return;
}


int ymo_http_response_is_ready(const ymo_http_response_t* response)
{
    return (response->flags & YMO_HTTP_RESPONSE_READY);
}


void ymo_http_response_finish(ymo_http_response_t* response)
{
    response->flags |= (YMO_HTTP_RESPONSE_READY | YMO_HTTP_RESPONSE_COMPLETE);
    ymo_conn_tx_enable(response->session->conn, 1);
    return;
}


int ymo_http_response_finished(const ymo_http_response_t* response)
{
    return (response->flags & YMO_HTTP_RESPONSE_COMPLETE);
}


static int should_buffer_body(ymo_http_response_t* response)
{
    const char* app_hdr_cl = ymo_http_hdr_table_get_id(
            &response->headers, HDR_ID_CONTENT_LENGTH);

    /* If the app set the content length, we don't need to buffer: */
    if( app_hdr_cl ) {
        return 0;
    }

    /* If content-length isn't set, but the exchange is complete, we can
     * calculate it:*/
    if( response->flags & YMO_HTTP_RESPONSE_COMPLETE ) {
        /* Sum the body bucket lengths: */
        ymo_bucket_t* body_data = response->body_head;
        while( body_data ) {
            response->content_len += body_data->len;
            body_data = body_data->next;
        }

        /* Set Content-Length header: */
        sprintf(response->content_len_str, "%zu", response->content_len);
#define USE_PRECOMPUTE
#ifdef USE_PRECOMPUTE
        ymo_http_hdr_table_insert_precompute(
                &response->headers,
                HDR_ID_CONTENT_LENGTH,
                "Content-Length",
                sizeof("Content-Length")-1,
                response->content_len_str);
#else
        ymo_http_hdr_table_insert(
                &response->headers, "Content-Length", response->content_len_str);
#endif
        ymo_log_debug("Content length for %p: %zu",
                response, response->content_len);
        return 0;
    } else if( response->flags & YMO_HTTP_FLAG_SUPPORTS_CHUNKED ) {
        /* If the response isn't complete, but we're allowed to chunk
         * the response, set Transfer-Encoding: chunked:
         */
        ymo_http_hdr_table_insert(
                &response->headers, "Transfer-Encoding", "Chunked");
        response->flags |= YMO_HTTP_RESPONSE_CHUNKED;
        return 0;
    }

    /* Otherwise, we've got no choice but to buffer the body data
     * before serializing it so we can calculate the content length
     * next time:
     */
    return 1;
}


ymo_bucket_t* ymo_http_response_start(
        ymo_conn_t* conn, ymo_http_response_t* response)
{
    /* If we don't have an HTTP status, bail with invalid.
     * TODO: this should HTTP 5xx something.
     */
    if( response->status_str[0] == '\0' || !response->status_len ) {
        errno = EINVAL;
        return NULL;
    }

    if( should_buffer_body(response) ) {
        ymo_log_debug("Buffering body data before serialization on %p",
                (void*)conn);
        errno = YMO_OKAY;
        return NULL;
    }

    /* TODO: we can do better than this. */
    ymo_bucket_t* bucket_out = NULL;
    ssize_t remain = YMO_HTTP_SEND_BUF_SIZE;
    char* response_buf = YMO_ALLOC(sizeof(char)*remain);
    if( !response_buf ) {
        ymo_log_debug("Unable to allocate response buffer of size %zu", remain);
        goto serialize_nomem;
    }
    char* insert = response_buf;

    /* Add status line: */
    if( remain < (ssize_t)(response->status_len + 11) ) {
        goto serialize_nomem;
    }

    /* Copy over the status string: */
    memcpy(insert, "HTTP/1.1 ", 9);
    insert += 9;
    memcpy(insert, response->status_str, response->status_len);
    insert += response->status_len;
    (*insert++) = '\r';
    (*insert++) = '\n';
    remain -= (11 + response->status_len);

    /* Headers:
     * TODO: ...it might be faster/cheaper just to make a bucket out of each,
     *       despite the waste for ':' and CRLF — ELSE, maybe have a padded
     *       bucket type with a header/trailer; ELSE, maybe store the ':' and
     *       CRLF at the time of key insertion in the header table.
     */
    const char* key;
    size_t key_len;
    const char* value;
    ymo_http_hdr_ptr_t iter = ymo_http_hdr_table_next(
            &response->headers, NULL, &key, &key_len, &value);
    while( iter )
    {
        /* Header name and ':' */
        if( (ssize_t)key_len < remain+2 ) {
            memcpy(insert, key, key_len);
            insert += key_len;
            remain -= key_len;

            (*insert++) = ':';
            (*insert++) = ' ';
            remain -= 2;
        } else {
            goto serialize_nomem;
        }

        /* Header value */
        char* terminal = memccpy(insert, value, '\0', remain);
        if( !terminal ) {
            goto serialize_nomem;
        }

        ssize_t copied = (--terminal - insert);
        insert += copied;
        remain -= copied;

        /* CRLF */
        if( remain >= 2 ) {
            *insert++ = '\r';
            *insert++ = '\n';
            remain -= 2;
        } else {
            goto serialize_nomem;
        }

        iter = ymo_http_hdr_table_next(
                &response->headers, iter, &key, &key_len, &value);
    }

    /* End of headers */
    if( remain < 2 ) {
        goto serialize_nomem;
    }

    *insert++ = '\r';
    *insert++ = '\n';
    remain -= 2;

    /* Attach the serialized HTTP exchange response and headers to the outgoing
     * bucket list:
     */
    bucket_out = ymo_bucket_create(NULL, NULL,
            response_buf, YMO_HTTP_SEND_BUF_SIZE,
            response_buf, insert-response_buf);
    if( bucket_out ) {
        return bucket_out;
    }

serialize_nomem:
    ymo_log_debug(
            "Out of memory serializing headers (%p)",
            (void*)conn);
    if( response_buf ) {
        YMO_FREE(response_buf);
    }
    errno = ENOMEM;
    return NULL;
}


ymo_bucket_t* ymo_http_response_body_get(
        ymo_conn_t* conn, ymo_http_response_t* response)
{
    static char chunk_hdr_buf[8];
    static const char* chunk_term = "\r\n";
    ymo_bucket_t* bucket_out = NULL;

    /* TODO: memory allocation/checking! ALSO: check for chunked support! */
    /* If the response isn't done, we're using transfer encoding chunked: */
    /* Move the body onto the outgoing buffer and set pointers to NULL. */
    if( response->body_head ) {
        int no_chars;
        if( response->flags & YMO_HTTP_RESPONSE_CHUNKED ) {
            ymo_log_trace("Chunked response for %p",
                    (void*)response);
            ymo_bucket_t* current = response->body_head;
            while( current ) {
                ymo_bucket_t* body_data = current;
                current = current->next;
                no_chars = snprintf(
                        chunk_hdr_buf, 7, "%zx\r\n", body_data->len);
                ymo_bucket_t* chunk_hdr = YMO_BUCKET_FROM_CPY(
                        chunk_hdr_buf, no_chars);
                chunk_hdr->next = body_data;
                body_data->next = YMO_BUCKET_FROM_CPY(
                        chunk_term, 2);
                if( bucket_out ) {
                    ymo_bucket_append(bucket_out, chunk_hdr);
                } else {
                    bucket_out = chunk_hdr;
                }
            }
        } else {
            ymo_log_trace("Unchunked response for %p",
                    (void*)response);
            bucket_out = response->body_head;
        }
        response->body_head = response->body_tail = NULL;
    }

    return bucket_out;
}


void ymo_http_response_free(ymo_http_response_t* response)
{
    if( response ) {
        ymo_http_hdr_table_clear(&response->headers);

        /* TODO: doc this + handler EAGAIN behavior! */
        if( response->exchange ) {
            ymo_http_exchange_free(response->exchange);
        }
    }
    YMO_DELETE(ymo_http_response_t, response);
    return;
}

