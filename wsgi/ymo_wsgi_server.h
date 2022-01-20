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



/** Server (IO Thread)
 * ===================
 *
 * Functions and types used to start up the libyimmo core server for WSGI.
 *
 *
 */

#ifndef YMO_WSGI_IO_H
#define YMO_WSGI_IO_H
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pthread.h>
#include <ev.h>

#include "ymo_http.h"
#include "core/ymo_net.h"

#include "yimmo_wsgi.h"


/**---------------------------------
 *            Setup
 *---------------------------------*/

/** Create a new yimmo HTTP server using :c:func:`ymo_http_simple_init`,
 * storing the current :c:type:`ymo_wsgi_proc_t` as the server's ``data``
 * attribute.
 */
ymo_server_t* ymo_wsgi_server_init(
        struct ev_loop* loop, in_port_t http_port, ymo_wsgi_proc_t* proc);

/** (At the moment, this is a no-op).
 */
ymo_status_t ymo_wsgi_server_start(struct ev_loop* loop);


/**---------------------------------
 *        Yimmo Callbacks
 *---------------------------------*/

/** HTTP header callback, invoked by ``libyimmo_http``.
 *
 * See also: :c:type:`ymo_http_header_cb_t`
 *
 */
ymo_status_t ymo_wsgi_server_header_cb(
        ymo_http_session_t* http_session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        void* user_data);

/** HTTP handler callback, invoked by ``libyimmo_http``.
 *
 * See also: :c:type:`ymo_http_cb_t`
 *
 */
ymo_status_t ymo_wsgi_server_request_cb(
        ymo_http_session_t* http_session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        void* user_data);


/**---------------------------------
 *     EV Watcher Callbacks
 *---------------------------------*/

/** We use an `ev_prepare_watcher`_ to check the worker thread output queue for
 * response data before entering the ev loop to wait on events.
 *
 */
void ymo_wsgi_server_prepare(struct ev_loop* loop, ev_prepare* w, int revents);

/** We use an `ev_check_watcher`_ to check the output queue for response data
 * when the event loop wakes back up after waiting on events.
 *
 */
void ymo_wsgi_server_check(struct ev_loop* loop, ev_check* w, int revents);


#ifdef YMO_WSGI_USE_IDLE_WATCHER

/* (Not used at the moment...)
 */
void ymo_wsgi_server_idle(struct ev_loop* loop, ev_idle* w, int revents);

#endif /* YMO_WSGI_USE_IDLE_WATCHER */

/** This is the callback function for the server's `ev_async_watcher`_, which is
 * used by the *worker thread* (using `ev_async_send`_) to notify the IO thread
 * that some response data is ready for processing.
 *
 */
void ymo_wsgi_server_async(struct ev_loop* loop, ev_async* w, int revents);

#endif /* YMO_WSGI_IO_H */


