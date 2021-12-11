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

/** # Example: HTTP Server
 *
 * A simple HTTP server with a single endpoint in two easy steps:
 *
 * 1. [Define an HTTP Callback](#1-define-an-http-callback)
 * 2. [Create and Start the Server](#2-create-and-start-the-server)
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

#include "start_info.h"

/* Default port to listen on for http connections. */
#define HTTP_PORT 8081


/*-------------------------------------------------------*
 *                     Prototypes:
 *-------------------------------------------------------*/

/* Stop the ev loop on sigint: */
static void sigint_handler(struct ev_loop* loop, ev_signal* w, int revents);


/** ## 1. Define an HTTP callback
 * User-space HTTP callback invoked by ymo_http when a new request is received
 * from an HTTP session.
 *
 * - `session`: the session is an HTTP protocol wrapper around the connection
 * - `request`: the yimmo object containing information on the request
 * - `response`: the response object to be filled out
 * - `user_data`: user data associated with this session (`NULL` here; see the API docs for more info)
 *
 * Return `YMO_OKAY` on success; a value from errno.h on error.
 *
 * > **NOTE:** Yimmo sends all _payload_ data (i.e. message _bodies_) using
 * > Apache-esque "bucket brigades" (`ymo_bucket_t`).
 *
 * For more info, see:
 *
 * - HTTP Guide: http://blog.yimmo.org/yimmo/guide/http.html
 * - HTTP API: http://blog.yimmo.org/yimmo/http/api.html
 *
 */
static ymo_status_t test_http_callback(
        ymo_http_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        void* user_data)
{
    ymo_log_info("HTTP: >%s %s<", request->method, request->uri);

    if( request->content_length ) {
        ymo_log_info("Got body data: %s", request->body);
    }

    ymo_http_response_insert_header(response, "content-type", "text/html");
    ymo_http_response_set_status_str(response, "200 OK");
    ymo_bucket_t* content = YMO_BUCKET_FROM_REF("OK", 2);

    ymo_http_response_body_append(response, content);
    ymo_http_response_finish(response);
    return YMO_OKAY;
}


/** ## 2. Create and Start the Server!
 */
int main(int argc, char** argv)
{
    ymo_server_t* http_srv = NULL;

    /* Say hello */
    issue_startup_msg();

    ymo_http_upgrade_handler_t* handlers[2] = {
        ymo_http2_no_upgrade_handler(),
        NULL,
    };

    /* libev loop init: */
    struct ev_loop* loop = ev_default_loop(0);

    /* Create the HTTP server: */
    http_srv = ymo_http_simple_init(
            loop, HTTP_PORT, &test_http_callback, handlers, NULL);

    if( !http_srv ) {
        ymo_log_error("Unable to create server: %s", strerror(errno));
        return -1;
    }

    /* Install a sigint handler so we can terminate gracefully: */
    ev_signal sigint_watcher;
    ev_signal_init(&sigint_watcher, sigint_handler, SIGINT);
    ev_signal_start(loop, &sigint_watcher);

    /* Start it up! */
    ymo_server_start(http_srv, loop);
    ev_run(loop,0);
    ymo_log(YMO_LOG_INFO, "Shutting down!");
    ymo_server_free(http_srv);
    return 0;
}


static void sigint_handler(struct ev_loop* loop, ev_signal* w, int revents)
{
    ev_break(loop, EVBREAK_ALL);
    return;
}

