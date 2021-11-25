/*=============================================================================
 *
 *  Copyright (c) 2014 Andrew Canaday
 *
 *  This file is part of libyimmo (sometimes referred to as "yimmo" or "ymo").
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


/** # Example: HTTP Server
 *
 * 1. [Define an HTTP Callback](#1-define-an-http-callback)
 * 2. [Create and Start the Server](#2-create-and-start-the-server)
 *
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

/** Default port to listen on for http connections. */
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
 * - `request`:  the yimmo object containing information on the request
 * - `response`:  the response object to be filled out
 *
 * Return `YMO_OKAY` on success; a value from errno.h on error.
 *
 * > **NOTE:** Yimmo sends all _payload_ data (i.e. message _bodies_) using
 * > Apache-style "bucket brigades" (`ymo_bucket_t`).
 *
 * Set the content type to "text/html" and set the body using EXAMPLE_TEXT.
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

    ymo_http_response_set_header(response, "content-type", "text/html");
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

