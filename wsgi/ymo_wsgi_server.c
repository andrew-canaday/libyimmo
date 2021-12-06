/*=============================================================================
 *
 *  Copyright (c) 2021 Andrew Canaday
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

/** # ymo_wsgi_server.c
 * Functions and types used to start up the libyimmo core server for WSGI.
 *
 *
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

/** Default TCP accept waitlist length. */
#define HTTP_DEFAULT_LISTEN_BACKLOG 100

#include "ymo_log.h"
#include "ymo_http.h"
#include "ymo_ws.h"
#include "ymo_http.h"
#include "ymo_log.h"

#include "ymo_wsgi_server.h"
#include "ymo_wsgi_worker.h"
#include "ymo_wsgi_session.h"
#include "ymo_wsgi_exchange.h"

#if defined(YIMMO_PY_WEBSOCKETS) && (YIMMO_PY_WEBSOCKETS == 1)
#include "ymo_py_websockets.h"
#endif /* YIMMO_PY_WEBSOCKETS */

/*---------------------------------*
 *        Server Functions:
 *---------------------------------*/
/* libyimmo_http receive callback */
ymo_status_t ymo_wsgi_server_request_cb(
        ymo_http_session_t* http_session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        void* user_data)
{
    ymo_log_debug("WSGI HTTP callback: %s", "REQUEST");
    ymo_status_t status = YMO_WOULDBLOCK;

    ymo_wsgi_session_t* wsgi_session = user_data;
    if( !wsgi_session ) {
        return EINVAL;
    }

    ymo_wsgi_worker_t* worker = ymo_wsgi_session_worker(wsgi_session);
    ymo_wsgi_worker_lock_in(worker);
    wsgi_session->head->response = response;
    ymo_wsgi_worker_add_exchange(worker, wsgi_session->head);
    ymo_wsgi_worker_notify(worker);
    ymo_wsgi_worker_unlock_in(worker);
    return status;
}


ymo_status_t ymo_wsgi_server_header_cb(
        ymo_http_session_t* http_session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        void* user_data)
{
    /* HACK HACK: skip everything of upgrade requests... */
    if( ymo_http_hdr_table_get(&request->headers, "upgrade") ) {
        return YMO_OKAY;
    }

    ymo_log_debug("WSGI HTTP callback: %s", "HEADER");
    ymo_status_t status = YMO_OKAY;
    ymo_wsgi_session_t* wsgi_session = user_data;
    if( !wsgi_session ) {
        return EINVAL;
    }

    ymo_wsgi_worker_t* worker = ymo_wsgi_session_worker(wsgi_session);
    ymo_wsgi_worker_lock_in(worker);
    ymo_wsgi_exchange_t* exchange = ymo_wsgi_session_create_exchange(
            wsgi_session, request, response);
    ymo_wsgi_exchange_incref(exchange); /* +1 for server */
    ymo_wsgi_worker_unlock_in(worker);

    if( !exchange ) {
        return ENOMEM;
    }
    return status;
}

ymo_server_t* ymo_wsgi_server_init(
        struct ev_loop* loop, in_port_t http_port, ymo_wsgi_proc_t* proc)
{
    /* Default HTTP config: */
    ymo_server_config_t http_cfg;
    memset(&http_cfg, 0, sizeof(http_cfg));
    http_cfg.loop = loop;
    http_cfg.port = http_port;
    http_cfg.flags = (YMO_SERVER_REUSE_ADDR | YMO_SERVER_REUSE_PORT);
    http_cfg.listen_backlog = HTTP_DEFAULT_LISTEN_BACKLOG;

    int n = 0;
    ymo_proto_t* http_proto = NULL;
    ymo_server_t* http_srv = NULL;

#if defined(YIMMO_PY_WEBSOCKETS) && (YIMMO_PY_WEBSOCKETS == 1)
    /* TODO: should be configurable, not installed by default!
     *
     * Initialize the websocket protocol: */
    ymo_proto_t* ws_proto = ymo_proto_ws_create(
            YMO_WS_SERVER_DEFAULT,
            &ymo_wsgi_ws_connect_cb,
            &ymo_wsgi_ws_recv_cb,
            &ymo_wsgi_ws_close_cb);
#endif /* YIMMO_PY_WEBSOCKETS */

    /* Initialize echo http_srv params: */
    http_proto = ymo_proto_http_create(
            &ymo_wsgi_session_init,
            &ymo_wsgi_server_request_cb,
            &ymo_wsgi_server_header_cb,
            NULL,
            &ymo_wsgi_session_cleanup,
            proc
            );

#if defined(YIMMO_PY_WEBSOCKETS) && (YIMMO_PY_WEBSOCKETS == 1)
    /* TODO: these should be configurable, not installed by default! */
    /* Websocket upgrade handler: */
    ymo_http_add_upgrade_handler(
        http_proto, ymo_ws_http_upgrade_handler(ws_proto));
    ymo_http_add_upgrade_handler(
        http_proto, ymo_http2_no_upgrade_handler());
#endif /* YIMMO_PY_WEBSOCKETS */

    if( !http_proto ) {
        goto http_init_bail;
    }

    /* Set up the yimmo http_srv: */
    http_srv = ymo_server_create(&http_cfg, http_proto);
    if( http_srv ) {
        if( (n = ymo_server_init(http_srv)) ) {
            ymo_log(YMO_LOG_ERROR, strerror(n));
            ymo_server_free(http_srv);
            http_srv = NULL;
        }
    }
    return http_srv;

http_init_bail:
    return NULL;
}


ymo_status_t ymo_wsgi_server_start(struct ev_loop* loop)
{
    return YMO_OKAY;
}


/** Invoked during check, prepare, and (if enabled) idle loop watchers. */
static void ymo_wsgi_server_queue_responses(
        struct ev_loop* loop, ymo_wsgi_worker_t* worker)
{
    ymo_wsgi_worker_lock_out(worker);
    if( !ymo_queue_size(&worker->queue_out) ) {
        /* YMO_WSGI_TRACE("Emtpy queue. Bailing. (watcher: %p)", (void*)w); */
        goto response_cb_done;
    }

    ymo_wsgi_exchange_t* exchange = NULL;
    while( (exchange = ymo_queue_popfront(&worker->queue_out)) ) {
        if( ymo_wsgi_session_is_closed(exchange->session) ) {
            YMO_WSGI_TRACE("Connection closed for exchange: %p", (void*)exchange);

            /* TODO: HACK HACK HACK */
            if( !exchange->sent ) {
                YMO_WSGI_TRACE("Exchange unset. Marking as sent and deleting ref: %p", (void*)exchange);
                goto resp_sent;
            }
            goto resp_decref;
        }

        if( exchange->body_data ) {
            YMO_WSGI_TRACE("Appending body data for exchange: %p", (void*)exchange);
            ymo_http_response_body_append(exchange->response, exchange->body_data);
            exchange->body_data = NULL;
        }

        if( exchange->done && !exchange->sent ) {
            YMO_WSGI_TRACE("Response finished for exchange: %p", (void*)exchange);
            ymo_http_response_finish(exchange->response);
resp_sent:
            exchange->sent = 1;
            ymo_wsgi_exchange_decref(exchange); /* -1 for server */
            ymo_wsgi_session_exchange_done(exchange->session);
        }
resp_decref:
        ymo_wsgi_exchange_decref(exchange); /* -1 for queue */
    }
response_cb_done:
    ymo_wsgi_worker_unlock_out(worker);
}

void ymo_wsgi_server_async(struct ev_loop* loop, ev_async* w, int revents)
{
    ymo_wsgi_server_queue_responses(loop, w->data);
}


void ymo_wsgi_server_prepare(struct ev_loop* loop, ev_prepare* w, int revents)
{
    ymo_wsgi_server_queue_responses(loop, w->data);
}


void ymo_wsgi_server_check(struct ev_loop* loop, ev_check* w, int revents)
{
    ymo_wsgi_server_queue_responses(loop, w->data);
#ifdef YMO_WSGI_USE_IDLE_WATCHER
    ymo_wsgi_worker_t* worker = w->data;
    ev_idle_start(loop, &worker->idle_watcher);
#endif /* YMO_WSGI_USE_IDLE_WATCHER */
}


#ifdef YMO_WSGI_USE_IDLE_WATCHER
void ymo_wsgi_server_idle(struct ev_loop* loop, ev_idle* w, int revents)
{
    ymo_wsgi_server_queue_responses(loop, w->data);
    ev_idle_stop(loop, w);
}
#endif /* YMO_WSGI_USE_IDLE_WATCHER */



