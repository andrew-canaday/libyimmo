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




#ifndef YMO_WEBSOCKETS_H
#define YMO_WEBSOCKETS_H
#include <stdint.h>
#include <errno.h>
#include <netinet/in.h>
#include <uuid/uuid.h>
#include <ev.h>
#include "yimmo.h"
#include "ymo_http.h"

/** WebSockets API
 * ================
 *
 * Yimmo WebSockets public API.
 *
 *
 * .. toctree::
 *   :hidden:
 *   :maxdepth: 3
 *
 *   index
 *
 * .. contents:: Contents
 *   :local:
 *
 */

/**---------------------------------------------------------------
 * Server Settings
 *---------------------------------------------------------------*/

/** Type for `flags` param passed in to :c:func:`ymo_proto_ws_create`.
 */
typedef uint8_t ymo_ws_server_flags_t;

/** The receive callback will be invoked with data, as
 * it is received from the client. Applications must
 * buffer their own multi-message payloads and check
 * for the FIN bit using :c:macro:`YMO_WS_FLAG_FIN`.
 *
 */
#define YMO_WS_SERVER_DEFAULT 0x0

/** The receive callback will be invoked with complete
 * messages. The WS module will buffer multi-part
 * messages for the application — up to
 * :c:macro:`YMO_WS_MSG_LEN_MAX` bytes — and
 * invoke the receive callback once the FIN bit is set.
 *
 */
#define YMO_WS_SERVER_BUFFERED 0x01

#ifndef YMO_WS_MSG_LEN_MAX

#ifndef YMO_WS_MSG_LEN_MAX

/** Max length of a *buffered* WS message.
 *
 * .. todo::
 *    Move this to runtime configuration!
 */
#  define YMO_WS_MSG_LEN_MAX 4096

#endif


#endif /* YMO_WS_MSG_LEN_MAX */

/**---------------------------------------------------------------
 * Message Header
 *---------------------------------------------------------------*/

/**
 * .. note::
 *    See `RFC6455: Framing <https://datatracker.ietf.org/doc/html/rfc6455#section-5>`_
 *    for more info.
 */

/** Byte 0
 * ~~~~~~~~
 */

/**  */
#define YMO_WS_FLAG_FIN  0x80

/*  */
#define YMO_WS_FLAG_RSV1 0x40

/*  */
#define YMO_WS_FLAG_RSV2 0x20

/*  */
#define YMO_WS_FLAG_RSV3 0x10

/*  */
#define YMO_WS_MASK_OP   0x0f

/** Indicates that the frame is a continuation frame.
 */
#define YMO_WS_OP_CONTINUATION 0x00

/** Indicates that the frame is a text frame.
 */
#define YMO_WS_OP_TEXT         0x01

/** Indicates that the frame is a binary frame.
 */
#define YMO_WS_OP_BINARY       0x02

/* 0x03-0x07 are reserved for further non-control frames */
/** Indicates that the frame is a connection close.
 */
#define YMO_WS_OP_CLOSE        0x08

/** Indicates that the frame is a ping.
 */
#define YMO_WS_OP_PING         0x09

/** Indicates that the frame is a pong.
 */
#define YMO_WS_OP_PONG         0x0a

/* 0x0b-0x0f are reserved for further control frames */

/**
 * Byte 1
 * ~~~~~~
 *
 */

/**  */
#define YMO_WS_FLAG_MASKED 0x80

/**  */
#define YMO_WS_MASK_LEN    0x7f


/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/** Opaque date type used to represent WebSocket connecctions
 *
 */
typedef struct ymo_ws_session ymo_ws_session_t;

/** WebSocket connect callback type.
 *
 * :session: a new websocket connection
 * :returns: ``NULL`` or a ``void*`` to custom data to associate with the session
 *
 */
typedef ymo_status_t (*ymo_ws_connect_cb_t)(ymo_ws_session_t* session);

/** WebSocket receive callback type — invoked for each packet of data.
 *
 * :session: the WebSocket session sending the message
 * :user_data: an optional pointer to user data associated with the session
 * :flags: flags from the incoming `Message Header`_
 * :msg: frame payload
 * :len: frame length
 * :returns: :c:macro:`YMO_OKAY` on success, errno.h value on failure
 *
 */
typedef ymo_status_t (*ymo_ws_recv_cb_t)(
        ymo_ws_session_t* session,
        void* user_data,
        uint8_t flags,
        const char* msg,
        size_t len);

/** WebSocket close callback type.
 *
 * :session: the session for the closed websocket
 * :user_data: an optional pointer to user data associated with the session
 *
 */
typedef void (*ymo_ws_close_cb_t)(ymo_ws_session_t* session, void* user_data);



/**---------------------------------------------------------------
 * Session Data
 *---------------------------------------------------------------*/

void ymo_ws_session_set_userdata(ymo_ws_session_t* session, void* user_data);
void* ymo_ws_session_get_userdata(ymo_ws_session_t* session);


/**---------------------------------------------------------------
 * Protocol Management
 *---------------------------------------------------------------*/

/** Create a WebSocket protocol object.
 *
 * :param flags: see `Server Settings`_
 * :param connect_cb: see :c:type:`ymo_ws_connect_cb_t`
 * :param recv_cb: see :c:type:`ymo_ws_recv_cb_t`
 * :param close_cb: see :c:type:`ymo_ws_close_cb_t`
 * :returns: :c:type:`ymo_proto_t` pointer.
 *
 */
ymo_proto_t* ymo_proto_ws_create(
        ymo_ws_server_flags_t flags,
        ymo_ws_connect_cb_t connect_cb,
        ymo_ws_recv_cb_t recv_cb,
        ymo_ws_close_cb_t close_cb);

/** Given a websocket protocol object, return a
 * :c:type:`ymo_http_upgrade_handler_t` which handles WebSocket upgrade
 * requests, which can be passed to :c:func:`ymo_http_simple_init`.
 *
 */
ymo_http_upgrade_handler_t* ymo_ws_http_upgrade_handler(ymo_proto_t* proto);

/** Send a message to the given websocket session.
 *
 * :param session: the websocket session
 * :param flags: flags for the outgoing `Message Header`_
 * :param payload: a :c:type:`ymo_bucket_t` encapsulating the data
 *     to be sent (**NOTE**: the payload can be a single bucket _or_ a chain
 *     of buckets).
 * :returns: :c:macro:`YMO_OKAY` on success; ``errno`` value on failure.
 *
 *
 * **Example:**
 *
 * .. code-block:: c
 *
 *    ymo_bucket_t* my_msg = YMO_BUCKET_FROM_REF("Hello, world!", 13);
 *    return ymo_ws_session_send(
 *            session, YMO_WS_FLAG_FIN | YMO_WS_OP_TEXT, my_msg);
 *
 */
ymo_status_t ymo_ws_session_send(
        ymo_ws_session_t* session,
        uint8_t flags,
        ymo_bucket_t* payload);

#endif /* YMO_WEBSOCKETS_H */




