/*============================================================================*
 * Copyright (c) 2021 Andrew T. Canaday
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


/*-----------------------------------------------------------------------------
 *
 * yimmo-ws-echo: Unadorned echo server used for Autobahn tests.
 *
 *----------------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include <ev.h>

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_http.h"
#include "ymo_ws.h"


#ifndef WS_ECHO_PORT
# define WS_ECHO_PORT 8081
#endif /* WS_ECHO_PORT */


#define OUT_FMT(fmt, ...) \
    fprintf(stderr, "%s: " fmt "\n", e_name, __VA_ARGS__)

#define OUT_MSG(msg) OUT_FMT("%s", msg)


/*----------------------------------------------------
 * Globals
 *----------------------------------------------------*/

static char* e_name  = NULL;
size_t no_connect    = 0;
size_t no_disconnect = 0;
size_t no_frames     = 0;
size_t no_msgs       = 0;


/*----------------------------------------------------
 * Callbacks:
 *----------------------------------------------------*/
static ymo_status_t echo_ws_connect_cb(ymo_ws_session_t* session)
{
    ++no_connect;
    return YMO_OKAY;
}


static ymo_status_t echo_ws_recv_cb(
        ymo_ws_session_t* session,
        void*             user_data,
        uint8_t flags,
        const char*       data,
        size_t len)
{
    ++no_frames;

    if( flags & YMO_WS_FLAG_FIN ) {
        ++no_msgs;
    }

    return ymo_ws_session_send(
            session, flags, YMO_BUCKET_FROM_CPY(data, len));
}


static void echo_ws_close_cb(ymo_ws_session_t* session, void* user_data)
{
    ++no_disconnect;
    return;
}


static ymo_status_t echo_http_callback(
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


/*----------------------------------------------------
 * Signal Handling:
 *----------------------------------------------------*/
static void echo_sig_handler(struct ev_loop* loop, ev_signal* w, int w_sig)
{
    ymo_log_debug("Got signal %i. Breaking.", w_sig);
    ev_break(loop, EVBREAK_ALL);
    return;
}


/*----------------------------------------------------
 * Entrypoint:
 *----------------------------------------------------*/
int main(int argc, char** argv)
{
    e_name = basename(argv[0]);

    OUT_MSG("\n\n----- Yimmo WS Echo Server -----");

    struct ev_loop* loop = ev_default_loop(0);
    ymo_log_init();

    ymo_proto_t* ws_proto = ymo_proto_ws_create(
            YMO_WS_SERVER_DEFAULT,
            &echo_ws_connect_cb,
            &echo_ws_recv_cb,
            &echo_ws_close_cb);

    ymo_http_upgrade_handler_t* upgrade_handlers[] = {
        ymo_ws_http_upgrade_handler(ws_proto), /* WS upgrade handler */
        ymo_http2_no_upgrade_handler(),        /* HTTP2 --> HTTP/1.1. */
        NULL,                                  /* sentinel */
    };

    ymo_server_t* http_srv = ymo_http_simple_init(
            loop, WS_ECHO_PORT, &echo_http_callback, upgrade_handlers, NULL);

    /* Set up some signal handlers: */
    ev_signal sigint_watcher;
    ev_signal sigchld_watcher;
    ev_signal_init(&sigint_watcher, echo_sig_handler, SIGINT);
    ev_signal_init(&sigchld_watcher, echo_sig_handler, SIGTERM);
    ev_signal_start(loop, &sigint_watcher);
    ev_signal_start(loop, &sigchld_watcher);

    ymo_server_start(http_srv, loop);
    if( http_srv ) {
        OUT_FMT("Starting echo server on port %i", WS_ECHO_PORT);
        ev_run(ev_default_loop(0), 0);
        ymo_server_free(http_srv);
    } else {
        ymo_log_fatal("Server failed to start: %s", strerror(errno));
    }
    OUT_MSG("Server stopped");

    OUT_MSG("Totals:");
    OUT_FMT("  #active:     %zu", no_connect-no_disconnect);
    OUT_FMT("  #connect:    %zu", no_connect);
    OUT_FMT("  #disconnect: %zu", no_disconnect);
    OUT_FMT("  #frames:     %zu", no_frames);
    OUT_FMT("  #msgs:       %zu", no_msgs);
    return 0;
}

