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



#ifndef YMO_HTTP_PARSE_H
#define YMO_HTTP_PARSE_H
#include "ymo_config.h"
#include <stddef.h>
#include "yimmo.h"
#include "ymo_http.h"
#include "core/ymo_trie.h"
#include "ymo_http_exchange.h"

/** Parser
 * ========
 *
 */

/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

/** Used to parse HTTP exchange line.
 *
 * :param request_in: HTTP exchange being parsed
 * :param buffer: input buffer from socket
 * :param len: number of bytes in the buffer from the last read event
 * :returns: number of bytes parsed on success, -1 on failure.
 */
ssize_t ymo_parse_http_request_line(
        ymo_http_exchange_t* request_in, const char* buffer, size_t len);


/** Used to parse CRLF between headers or at end of headers.
 *
 * :param request_in: HTTP exchange being parsed
 * :param buffer: input buffer from socket
 * :param len: number of bytes in the buffer from the last read event
 *
 * :returns: number of bytes parsed on success, -1 on failure.
 */
ssize_t ymo_parse_http_crlf(
        ymo_http_exchange_t* request_in, const char* buffer, size_t len);


/** Used to parse HTTP headers.
 *
 * :param request_in: HTTP exchange being parsed
 * :param buffer: input buffer from socket
 * :param len: number of bytes in the buffer from the last read event
 *
 * :returns: number of bytes parsed on success, -1 on failure.
 */
ssize_t ymo_parse_http_headers(
        ymo_http_cb_t header_cb,
        ymo_http_session_t* session,
        ymo_http_exchange_t* request_in,
        const char* buffer,
        size_t len);


/** Used to parse HTTP body.
 *
 * :param body_cb: userspace body-data callabck (optional)
 * :param session: http session object (required if body_cb != NULL)
 * :param request_in: HTTP exchange being parsed
 * :param buffer: input buffer from socket
 * :param len: number of bytes in the buffer from the last read event
 *
 * :returns: number of bytes parsed on success, -1 on failure.
 */
ssize_t ymo_parse_http_body(
        ymo_http_body_cb_t body_cb,
        ymo_http_session_t* session,
        ymo_http_exchange_t* exchange,
        const char* buffer,
        size_t len);

#endif /* YMO_HTTP_PARSE_H */



