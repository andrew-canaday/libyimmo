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




#ifndef YMO_WS_PARSE_H
#define YMO_WS_PARSE_H
#include "yimmo_config.h"
#include <stddef.h>
#include "yimmo.h"
#include "ymo_ws.h"
#include "ymo_ws_session.h"

/** Parser
 * ========
 *
 */


/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

/** Parse websocket op code
 */
ssize_t ymo_ws_parse_op(ymo_ws_session_t* session, char* buffer, size_t len);

/** Parse websocket message length
 */
ssize_t ymo_ws_parse_len(ymo_ws_session_t* session, char* buffer, size_t len);

/** Parse websocket message length
 */
ssize_t ymo_ws_parse_len_ext(ymo_ws_session_t* session, char* buffer, size_t len);

/** Parse websocket message masking key
 */
ssize_t ymo_ws_parse_masking_key(ymo_ws_session_t* session, char* buffer, size_t len);

/** Parse websocket payload
 */
ssize_t ymo_ws_parse_payload(ymo_ws_session_t* session, char* buffer, size_t len);

#endif /* YMO_WS_PARSE_H */


