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




#ifndef YMO_HTTP_EXCHANGE_H
#define YMO_HTTP_EXCHANGE_H
#include "ymo_config.h"
#include "yimmo.h"
#include "ymo_http.h"

/** Exchange
 * ==========
 *
 */

/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/** Enum type used to track HTTP exchange parse state.
 */
YMO_ENUM8_TYPEDEF(http_state) {
    HTTP_STATE_CONNECTED,
    HTTP_STATE_REQUEST_METHOD,
    HTTP_STATE_REQUEST_URI_PATH,
    HTTP_STATE_REQUEST_QUERY,
    HTTP_STATE_REQUEST_FRAGMENT,
    HTTP_STATE_REQUEST_VERSION,
    HTTP_STATE_CRLF,
    HTTP_STATE_HEADER_NAME,
    HTTP_STATE_HEADER_VALUE_LEADING_SPACE,
    HTTP_STATE_HEADER_VALUE,
    HTTP_STATE_EXPECT,
    HTTP_STATE_BODY_CHUNK_HEADER,
    HTTP_STATE_BODY,
    HTTP_STATE_BODY_CHUNK_TRAILER,
    HTTP_STATE_COMPLETE,
} YMO_ENUM8_AS(http_state_t);


/** Enum type used to track HTTP end of line state.
 */
YMO_ENUM8_TYPEDEF(crlf_state) {
    HTTP_EXPECT_CRLF,
    HTTP_EXPECT_LF,
} YMO_ENUM8_AS(crlf_state_t);

/** Struct used to store HTTP exchange data.
 *
 * .. TODO::
 *    Move parse info stuff to separate struct and move to ymo_http_parse.
 *
 */
struct ymo_http_exchange {
    /* Request: */
    struct ymo_http_request  request;

    /* Parse info: */
    union {
        /* These are used for mutually exclusive parse phases: */
        struct {
            char*              hdr_name;
            char*              hdr_value;
            ymo_http_hdr_id_t  h_id;
        };
        struct {
            char*   chunk_current;
            char    chunk_hdr[6];
            size_t  body_remain;
        };
    };

    http_state_t  state;
    http_state_t  next_state;
    char*         recv_current;
    size_t        remain;
    char          recv_buf[YMO_HTTP_RECV_BUF_SIZE];

    /* Response:
     *
     * (TODO: just add this to the exchange and skip the malloc/free).
     */
    ymo_http_response_t* response;
};

/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

/** Create a new http exchange object.
 *
 * :returns: pointer to a new exchange, or NULL on failure.
 */
ymo_http_exchange_t* ymo_http_exchange_create(void);

/** Reset an HTTP exchange struct to handle a new incoming exchange
 */
void ymo_http_exchange_reset(ymo_http_exchange_t* exchange);

/** Free an http exchange object.
 *
 * :param exchange: exchange to free
 */
void ymo_http_exchange_free(ymo_http_exchange_t* exchange);

/** Used to get common HTTP exchange traits
 *
 * :param exchange: the HTTP exchange instance to query
 * :returns: ymo_http_flags_t type indicating exchange traits
 */
ymo_http_flags_t ymo_http_request_flags(const ymo_http_exchange_t* exchange);

#endif /* YMO_HTTP_EXCHANGE_H */



