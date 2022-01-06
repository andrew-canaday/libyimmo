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
#include <assert.h>
#include <errno.h>

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_alloc.h"
#include "ymo_http_session.h"
#include "ymo_http_exchange.h"
#include "ymo_http_response.h"

/*---------------------------------------------------------------*
 *  HTTP Request/Response Management:
 *---------------------------------------------------------------*/
ymo_http_session_t*
ymo_http_session_create(ymo_conn_t* conn)
{
    ymo_http_session_t* http_session = NULL;
    http_session = YMO_NEW(ymo_http_session_t);
    if( http_session ) {
        http_session->conn = conn;
        http_session->exchange = NULL;
        http_session->response = NULL;
        http_session->send_buffer = NULL;
    }
    return http_session;
}


void
ymo_http_session_free(ymo_http_session_t* http_session)
{
    if( http_session ) {
        ymo_conn_tx_enable(http_session->conn, 0);
        ymo_conn_rx_enable(http_session->conn, 0);
        ymo_log_trace("Freeing session: %p", (void*)http_session);
        ymo_http_session_free_request(http_session);
        ymo_status_t status;
        do
        {
            status = ymo_http_session_complete_response(http_session);
        } while( status == YMO_OKAY );
        ymo_bucket_free_all(http_session->send_buffer);
        YMO_DELETE(ymo_http_session_t, http_session);
    }
    return;
}


ymo_status_t
ymo_http_session_add_new_http_request(ymo_http_session_t* http_session)
{
    ymo_http_exchange_t* exchange = NULL;
    exchange = ymo_http_exchange_create();
    if( !exchange ) {
        return errno;
    }

    /* Initialize it: */
    ymo_log_trace("Creating exchange (%p)", (void*)exchange);
    http_session->exchange = exchange;
    return YMO_OKAY;
}


ymo_status_t
ymo_http_session_free_request(ymo_http_session_t* http_session)
{
    /* Pop the first guy off the list: */
    ymo_http_exchange_t* exchange = http_session->exchange;

    /* Free it, if present: */
    if( exchange ) {
        ymo_log_trace("Freeing exchange (%p)", (void*)exchange);
        http_session->exchange = NULL;
        ymo_http_exchange_free(exchange);
        return YMO_OKAY;
    }
    return EINVAL;
}


ymo_status_t ymo_http_session_init_response(
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
            ymo_http_hdr_table_insert_precompute(
                    &response->headers, HDR_ID_CONNECTION,
                    "Connection", sizeof("Connection")-1, "Keep-alive");
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


ymo_status_t ymo_http_session_add_response(
        ymo_http_session_t* http_session, ymo_http_response_t* response_in)
{
    ymo_log_trace(
            "Adding new response to HTTP session (%p)", (void*)http_session);
    if( http_session->response ) {
        ymo_http_response_t* last = http_session->response;
        while( last->next ) {
            last = last->next;
        }

        last->next = response_in;
    } else {
        http_session->response = response_in;
    }
    return YMO_OKAY;
}


ymo_http_response_t* ymo_http_session_next_response(
        ymo_http_session_t* http_session)
{
    return http_session->response;
}


ymo_status_t ymo_http_session_complete_response(
        ymo_http_session_t* http_session)
{
    /* Pop the first guy off the list: */
    ymo_http_response_t* response_out = http_session->response;

    /* Free it, if present: */
    if( response_out ) {
        ymo_log_trace("Freeing response_out (%p)", (void*)response_out);
        http_session->response = response_out->next;
        ymo_log_trace("New response_out: (%p)", (void*)http_session->response);
        ymo_http_response_free(response_out);
        return YMO_OKAY;
    }
    return EINVAL;
}


ymo_status_t ymo_http_session_is_read_ready(ymo_http_session_t* http_session)
{
    ymo_status_t status = YMO_OKAY;

    if( !http_session->exchange ) {
        status = ymo_http_session_add_new_http_request(http_session);
    }
    return status;
}


void ymo_http_session_set_userdata(ymo_http_session_t* session, void* data)
{
    session->user_data = data;
}


void* ymo_http_session_get_userdata(const ymo_http_session_t* session)
{
    return session->user_data;
}

