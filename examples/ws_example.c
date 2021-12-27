/*============================================================================*
 * Copyright (c) 2014 Andrew T. Canaday
 *
 * NOTICE: THIS EXAMPLE FILE IS LICENSED SEPARATELY FROM THE REST OF LIBYIMMO.
 *
 * This file is licensed under the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 *----------------------------------------------------------------------------*/

/** # Example: WebSocket Echo Server
 *
 * A simple WebSocket echo server in three easy steps:
 *
 *  1. [Define some WebSocket event callbacks](#1-websocket-protocol-callbacks)
 *  2. [Add a default HTTP handler](#2-add-an-http-callback-for-non-upgrade-requests)
 *  3. [Create and start the server](#3-create-and-start-the-server)
 *
 * *Fin!*
 *
 * Let's have a peek:
 * <br />
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ev.h>
#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_http.h"
#include "ymo_ws.h"


#include "start_info.h"

/* Default port to listen on for http connections. */
#ifndef HTTP_PORT
# define HTTP_PORT 8081
#endif /* HTTP_PORT */

/** ## 1. WebSocket Protocol Callbacks
 *
 * To start things off, we'll define our callback functions for the following
 * WebSockets events:
 *
 * - connect (analogous to `onopen`)
 * - receive (analogous to `onmessage`)
 * - close (analogous to `onclose`)
 *
 *
 */

/* WS connect callback: issue greeting */
static ymo_status_t test_ws_connect_cb(ymo_ws_session_t* session);

/* WS receive callback: echo payload to client */
static ymo_status_t test_ws_recv_cb(
        ymo_ws_session_t* session,
        void*             user_data,
        uint8_t flags,
        const char*       data,
        size_t len);

/* WS close callback: log close */
static void test_ws_close_cb(ymo_ws_session_t* session, void* user_data);


/** ### Connect Callback
 *
 * The connection callback takes a single argument, a `ymo_ws_session_t` (_all
 * WebSockets event callbacks provide the session as the first argument_ — see
 * API docs for more info). On success, the callback should return `YMO_OKAY`.
 * Otherwise, the connection is closed, and the return value is used to set
 * `errno`.
 *
 * > **NOTE:** Yimmo sends all _payload_ data (i.e. message _bodies_) using
 * > Apache-esque "bucket brigades" (`ymo_bucket_t`).
 *
 */
static ymo_status_t test_ws_connect_cb(ymo_ws_session_t* session)
{
    ymo_log_info("New WebSocket session: %p!", (void*)session);

#if defined(WS_EXAMPLE_ISSUE_HELLO) && WS_EXAMPLE_ISSUE_HELLO
    /* Encapsulate a string literal in a bucket: */
    ymo_bucket_t* my_msg = YMO_BUCKET_FROM_REF("Hello!", 6);

    /* Send a text-type message from with the FIN bit set: */
    return ymo_ws_session_send(
            session, YMO_WS_FLAG_FIN | YMO_WS_OP_TEXT, my_msg);
#else
    return YMO_OKAY;
#endif /* WS_EXAMPLE_ISSUE_HELLO */
}


/** ### Receive Callback
 *
 * This is invoked whenever a message is received from the client. In addition
 * to the session object, we get `flags` (the first 8 bits of the RFC6455
 * message frame), `data` (the raw data as `const char*`), and `len` the total
 * payload length. On success, the callback should return `YMO_OKAY`. Otherwise,
 * the connection is closed, with the return value being used to set `errno`.
 *
 */
static ymo_status_t test_ws_recv_cb(
        ymo_ws_session_t* session,
        void*             user_data,
        uint8_t flags,
        const char*       data,
        size_t len)
{
#if EXAMPLE_PRINT_MSG
    if( data && len ) {
        if( flags & YMO_WS_OP_TEXT ) {
            ymo_log_info("Recv from %p: \"%.*s\"",
                    (void*)session, (int)len, data);
        }
    } else {
        ymo_log_info(
                "Got a message with a zero length payload "
                "(allowed by RFC-6455!)");
    }
#endif

    return ymo_ws_session_send(
            session, flags, YMO_BUCKET_FROM_CPY(data, len));
}


/** > **Heads up! — Message Framing:**
 * >
 * > Since we're just echoing the payload back, we don't bother
 * > checking or setting the `OP_TYPE` or `FIN` bits — we just copy them back,
 * > as-is, to the client the way they were recieved.
 * >
 * > If you wanted to do something with the complete message — e.g.  parse some
 * > JSON payload with a non-streaming parser, etc — you'd want to check the FIN
 * > bit and buffer the incoming frames until you got to the end, or use a
 * > buffered connection adapter.
 *
 */


/** ### Close Callback
 *
 * The close callback is invoked when the WebSocket session has been terminated
 * (by either the client or server).
 *
 * > **WARNING:** _The `ymo_ws_session_t*` passed in as a parameter here cannot be
 * > meaningfully dereferenced after this function returns!_
 *
 */
static void test_ws_close_cb(ymo_ws_session_t* session, void* user_data)
{
    ymo_log_info("Session %p closed!", (void*)session);
    return;
}


/** <br /> */

/** ## 2. Add an HTTP callback for non-upgrade requests
 *
 * For kicks, we add a simple HTTP callback to serve a plain-text 200 response
 * to any HTTP 1.0, 1.1 requests which do not attempt a connection upgrade or
 * HTTP2 upgrade requests (which are defaulted back to HTTP 1.1 via
 * `ymo_http2_no_upgrade_handler` in `main`):
 *
 * For a deeper dive into what's going on here, see the HTTP module docs.
 *
 */
static ymo_status_t test_http_callback(
        ymo_http_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        void* user_data)
{
    ymo_http_response_insert_header(response, "content-type", "text/plain");
    ymo_bucket_t* content = YMO_BUCKET_FROM_REF("OK", 2);
    ymo_http_response_set_status_str(response, "200 OK");
    ymo_http_response_body_append(response, content);
    ymo_http_response_finish(response);
    return YMO_OKAY;
}


/** <br /> */

/** ## 3. Create and Start the Server!
 *
 * What we do here is:
 *
 *  1. Get a handle to the default libev event loop using `ev_default_loop`.
 *  2. Invoke `ymo_proto_ws_create` to create a new instance of the libyimmo
 *     WebSockets protocol, registering our connect, receive, and close
 *     callbacks.
 *  3. Add some HTTP upgrade handlers:
 *     - `ymo_ws_http_upgrade_handler` is the default HTTP->RFC6455 upgrade
 *       handler (you can roll your own, if you like; see the `ymo_http` module
 *       API docs for more info).
 *     - `ymo_http2_no_upgrade_handler` is a "fallback" handler which reverts
 *        HTTP2 upgrade attempts to HTTP/1.1
 *  4. Instantiate a libyimmo HTTP server using `ymo_http_simple_init`,
 *     registering our test HTTP callback as the principle handler and our
 *     "upgrade handler" chain (which contains the WebSockets upgrade handler
 *     + protocol).
 *  5. Run the thing!
 */
int main(int argc, char** argv)
{
    /* Say hello */
    issue_startup_msg();

    struct ev_loop* loop = ev_default_loop(0);

    /* Create a websocket protocol object: */
    ymo_proto_t* ws_proto = ymo_proto_ws_create(
            YMO_WS_SERVER_DEFAULT,
            &test_ws_connect_cb,
            &test_ws_recv_cb,
            &test_ws_close_cb);

    /* Set up an array of HTTP "Upgrade" upgrade_handlers: */
    ymo_http_upgrade_handler_t* upgrade_handlers[] = {
        ymo_ws_http_upgrade_handler(ws_proto), /* WS upgrade handler */
        ymo_http2_no_upgrade_handler(),        /* HTTP2 --> HTTP/1.1. */
        NULL,                                  /* sentinel */
    };

    /* Create the HTTP server: */
    ymo_server_t* http_srv = ymo_http_simple_init(
            loop, HTTP_PORT, &test_http_callback, upgrade_handlers, NULL);

    /* Run it! */
    ymo_server_start(http_srv, loop);
    if( http_srv ) {
        ev_run(ev_default_loop(0), 0);
        ymo_log(YMO_LOG_INFO, "Loop exited. Shutting down!");
        ymo_server_free(http_srv);
    } else {
        ymo_log(YMO_LOG_ERROR, "Server failed to start!");
    }
    return 0;
}

