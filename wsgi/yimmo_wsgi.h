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



#ifndef YIMMO_WSGI_H
#define YIMMO_WSGI_H

/** Common Declarations
 * =====================
 *
 * Miscellaneous types and utilities
 *
 *
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ev.h>
#include "ymo_http.h"

/* TODO: provide workaround when no atomic support! */
#ifndef __STDC_NO_ATOMICS__
#include <stdatomic.h>
#else
#error "stdatomic.h is required to build yimmo-wsgi"
#endif

/**---------------------------------
 *          Definitions
 *---------------------------------*/


/** Default number of process workers.
 *
 */
#ifndef YIMMO_WSGI_NO_PROC
#define YIMMO_WSGI_NO_PROC 1
#endif /* YIMMO_WSGI_NO_PROC */

/** Maximum number of process worker restarts before we give up.
 *
 */
#ifndef YMO_WSGI_PROC_MAX_RESTARTS
# define YMO_WSGI_PROC_MAX_RESTARTS 2
#endif /* YMO_WSGI_PROC_MAX_RESTARTS */

/** Default number of WSGI threads to run.
 *
 */
#ifndef YIMMO_WSGI_NO_THREADS
# define YIMMO_WSGI_NO_THREADS 1
#endif /* YIMMO_WSGI_NO_THREADS */

#define YMO_WSGI_TRACE_WSGI 1
#if defined(YMO_WSGI_TRACE_WSGI) && YMO_WSGI_TRACE_WSGI == 1
#define YMO_WSGI_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__)
#else
#define YMO_WSGI_TRACE(fmt, ...) ((void)(0))
#endif /* YMO_WSGI_TRACE_WSGI */

/** Number of queue nodes preallocated per thread worker.
 */
#define YMO_WSGI_WORKER_QUEUE_POOL_SIZE 100

/** Number of exchange structs to preallocated per session.
 */
#define YMO_WSGI_EXCHANGE_POOL_SIZE     2

/** cast a C yimmo context to a Python object pointer.
 */
#define YIMMO_CONTEXT_PY(ptr) ((PyObject*)(ptr))

/** cast a Python object pointer to a C yimmo context.
 */
#define YIMMO_CONTEXT_C(ptr) ((yimmo_context_t*)(ptr))

/** cast a C yimmo websocket to a Python object pointer.
 */
#define YIMMO_WEBSOCKET_PY(ptr) ((PyObject*)(ptr))

/** cast a Python object pointer to a C yimmo websocket.
 */
#define YIMMO_WEBSOCKET_C(ptr) ((yimmo_websocket_t*)(ptr))

#define YMO_WSGI_EXCHANGE_ATOMIC 1

/**---------------------------------
 *           Types
 *---------------------------------*/

/** Encapsulates the data for a single yimmo-wsgi _thread_ worker.
 */
typedef struct ymo_wsgi_worker ymo_wsgi_worker_t;

/** Encapsulate the data for a single yimmo-wsgi HTTP session.
 */
typedef struct ymo_wsgi_session ymo_wsgi_session_t;

/** Encapsulates the data for a single yimmo-wsgi exchange — i.e. a
 * request/response pair.
 */
typedef struct ymo_wsgi_exchange ymo_wsgi_exchange_t;

/** Encapsulates the "WSGI view" of a session, ``environ``, ``start_response``,
 * etc. For more info, see PEP3333.
 */
typedef struct yimmo_context yimmo_context_t;


/** A Yimmo Python WebSocket connection. */
typedef struct yimmo_websocket yimmo_websocket_t;


/** Encapsulates the data for a single yimmo-wsgi _process_ worker.
 */
typedef struct ymo_wsgi_proc ymo_wsgi_proc_t;


/** Pool type used to minimize ``malloc``/``free`` calls for sessions which
 * involve multiple exchanges.
 */
typedef struct ymo_wsgi_exchange_pool ymo_wsgi_exchange_pool_t;

typedef void (*ymo_prepare_cb)(
        struct ev_loop* loop, ev_prepare* w, int revents);

typedef void (*ymo_check_cb)(
        struct ev_loop* loop, ev_check* w, int revents);

typedef void (*ymo_async_cb)(
        struct ev_loop* loop, ev_async* w, int revents);

#ifdef YMO_WSGI_USE_IDLE_WATCHER
typedef void (*ymo_idle_cb)(
        struct ev_loop* loop, ev_idle* w, int revents);
#endif /* YMO_WSGI_USE_IDLE_WATCHER */


/*---------------------------------
 *            Globals
 *---------------------------------*/
extern const char* script_name;

#endif /* YIMMO_WSGI_H */


