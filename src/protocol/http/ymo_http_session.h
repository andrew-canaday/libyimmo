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




#ifndef YMO_HTTP_SESSION_H
#define YMO_HTTP_SESSION_H
#include "ymo_config.h"
#include "yimmo.h"
#include "ymo_http.h"

/** Session
 * =========
 *
 */

/**---------------------------------------------------------------
 * Data Types:
 *---------------------------------------------------------------*/

/** Internal structure used to manage a yimmo session.
 */
struct ymo_http_session {
    ymo_conn_t*          conn;
    ymo_http_exchange_t* exchange;
    ymo_http_response_t* response;
    ymo_bucket_t*        send_buffer;
    void*                user_data;
};


/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

/** */
ymo_http_session_t* ymo_http_session_create(ymo_conn_t* conn);


/** */
void ymo_http_session_free(ymo_http_session_t* session);


/** */
ymo_status_t ymo_http_session_add_new_http_request(
        ymo_http_session_t* http_session);


/** */
ymo_status_t ymo_http_session_free_request(
        ymo_http_session_t* http_session);


/** */
ymo_status_t ymo_http_session_add_response(
        ymo_http_session_t* http_session,
        ymo_http_response_t* response_in);


/** */
ymo_http_response_t* ymo_http_session_next_response(
        ymo_http_session_t* http_session);


/** */
ymo_status_t ymo_http_session_complete_response(
        ymo_http_session_t* http_session);


/** */
ymo_status_t ymo_http_session_is_read_ready(ymo_http_session_t* http_session);


#endif /* YMO_HTTP_SESSION_H */
