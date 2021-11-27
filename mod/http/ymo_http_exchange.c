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
#include <string.h>

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_alloc.h"
#include "ymo_http_exchange.h"
#include "ymo_http_hdr_table.h"

ymo_http_exchange_t* ymo_http_exchange_create(void)
{
    ymo_http_exchange_t* exchange = YMO_NEW0(ymo_http_exchange_t);
    if( exchange ) {
        ymo_http_exchange_reset(exchange);
        ymo_http_hdr_table_init(&exchange->request.headers);

        exchange->request.ws = ymo_blalloc_create(YMO_HTTP_REQ_WS_SIZE);
        if( !exchange->request.ws ) {
            ymo_http_exchange_free(exchange);
            exchange = NULL;
            errno = ENOMEM;
        }

        exchange->h_id = YMO_HTTP_HDR_HASH_INIT();
        exchange->request.content_length = 0;
        exchange->recv_current = exchange->recv_buf;
        exchange->request.method = exchange->recv_buf;
        exchange->state = HTTP_STATE_CONNECTED;
        exchange->remain = YMO_HTTP_RECV_BUF_SIZE;
    } else {
        errno = ENOMEM;
    }

    return exchange;
}

void ymo_http_exchange_reset(ymo_http_exchange_t* exchange)
{
    /*------------------------------------------*
     *           TODO: TEMP HACK:               *
     *------------------------------------------*/
    /* ymo_http_request_t: */
    exchange->request.method = exchange->recv_buf;
    exchange->request.uri = NULL;
    exchange->request.version = NULL;
    exchange->request.query = NULL;
    exchange->request.fragment = NULL;
    exchange->request.content_length = 0;
    ymo_http_hdr_table_clear(&exchange->request.headers);

    if( exchange->request.ws ) {
        ymo_blalloc_reset(exchange->request.ws);
    }

    /* ymo_http_exchange_t: */
    exchange->hdr_name = exchange->hdr_value = NULL;
    exchange->h_id = YMO_HTTP_HDR_HASH_INIT();
    exchange->request.content_length = 0;
    exchange->request.body_received = 0;
    exchange->recv_current = exchange->recv_buf;
    exchange->flags = 0;
    exchange->state = HTTP_STATE_CONNECTED;
    exchange->next_state = 0;
    exchange->remain = YMO_HTTP_RECV_BUF_SIZE;
    return;
}

ymo_http_flags_t ymo_http_request_flags(const ymo_http_exchange_t* exchange)
{
    return exchange->flags;
}

void ymo_http_exchange_free(ymo_http_exchange_t* exchange)
{
    if( exchange ) {
        if( exchange->request.ws ) {
            ymo_blalloc_free(exchange->request.ws);
        }

        if( exchange->request.body ) {
            /* HACK: make the limit configurable! */
#define HTTP_MAX_BODY 4096
            YMO_FREE(exchange->request.body);
        }
    }
    YMO_DELETE(ymo_http_exchange_t, exchange);
    return;
}



