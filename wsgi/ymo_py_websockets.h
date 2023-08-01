/*=============================================================================
 *
 * Copyright (c) 2021 Andrew Canaday
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




/** Python Websockets
 * ===================
 *
 */

#ifndef YMO_PY_WEBSOCKETS_H
#define YMO_PY_WEBSOCKETS_H

#include "yimmo.h"
#include "ymo_ws.h"

#include "yimmo_wsgi.h"

#define INIT_WEBSOCKETS_DOC \
        PyDoc_STR( \
        "init_websockets(on_open, on_message, on_close)\n" \
        "--\n\n" \
        "Enable websockets upgrades and define callbacks for \n" \
        "connect, message, and disconnect events.\n" \
        "\n" \
        "Keyword Arguments:\n" \
        "\n" \
        "  on_open    -- callable (or None) invoked on connect.\n" \
        "  on_message -- callable (or None) invoked on message.\n" \
        "  on_close   -- callable (or None) invoked on close.\n" \
        "\n" \
        "Callbacks:\n" \
        "\n" \
        "  The on_open and on_close functions must accept a single\n" \
        "  argument, ``ws``, which is a yimmo.WebSocket object.\n" \
        "\n" \
        "  The on_message function must accept three arguments:\n" \
        "  ``ws``    the yimmo.WebSocket object\n" \
        "  ``msg``   the websockets message payload as bytes\n" \
        "  ``flags`` websocket message flags as an int\n\n" \
        )


extern PyTypeObject yimmo_WebSocketType;
extern PyMethodDef yimmo_WebSocketType_methods[];

ymo_status_t ymo_wsgi_ws_connect_cb(ymo_ws_session_t* session);

ymo_status_t ymo_wsgi_ws_recv_cb(
        ymo_ws_session_t* session,
        void*             user_data,
        uint8_t flags,
        const char*       data,
        size_t len);

void ymo_wsgi_ws_close_cb(ymo_ws_session_t* session, void* user_data);

PyObject* yimmo_init_websockets(
        PyObject* self, PyObject* args, PyObject* kwargs);

#endif /* YMO_PY_WEBSOCKETS_H */
