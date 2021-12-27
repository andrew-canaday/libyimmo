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




#ifndef YMO_HTTP_RESPONSE_H
#define YMO_HTTP_RESPONSE_H
#include "ymo_config.h"

#include "yimmo.h"
#include "core/ymo_bucket.h"
#include "ymo_http_session.h"
#include "core/ymo_conn.h"
#include "ymo_http.h"
#include "ymo_http_exchange.h"
#include "ymo_http_hdr_table.h"

#define STATUS_STR_BUFF_SIZE 32
#define STATUS_STR_MAX_LEN   (STATUS_STR_BUFF_SIZE-1)

/** Response
 * ==========
 *
 */

/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/** HTTP response structure.
 *
 * .. todo::
 *   Should it just be one buffer for the whole status line?
 *
 */
struct ymo_http_response {
    ymo_http_session_t*       session;
    ymo_http_exchange_t*      exchange;
    struct ymo_http_response* next;
    ymo_http_hdr_table_t      headers;
    size_t                    content_len;
    char                      content_len_str[21];
    size_t                    status_len;
    char                      status_str[STATUS_STR_BUFF_SIZE];
    ymo_bucket_t*             body_head;
    ymo_bucket_t*             body_tail;
    ymo_proto_t*              proto_new;
    ymo_http_status_t         status;
    ymo_http_flags_t          flags;

};

/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

/** Create a new http response object.
 *
 * :returns: pointer to new instance on success; NULL on failure
 */
ymo_http_response_t* ymo_http_response_create(ymo_http_session_t* session);

/** Serialize an http response headers into a send-able string.
 *
 */
ymo_bucket_t* ymo_http_response_start(
        ymo_conn_t* conn, ymo_http_response_t* response);

/** Retrieve whatever body data we have, adding chunk headers, if need be.
 */
ymo_bucket_t* ymo_http_response_body_get(
        ymo_conn_t* conn, ymo_http_response_t* response);

/** Free an http response object.
 *
 * :param response: HTTP response to free.
 */
void ymo_http_response_free(ymo_http_response_t* response);

#endif /* YMO_HTTP_RESPONSE_H */



