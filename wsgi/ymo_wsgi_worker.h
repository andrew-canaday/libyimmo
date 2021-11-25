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



/** Thread Worker
 * ===================
 *
 * Threading, locking, and handoff between libev and python
 *
 *
 */

#ifndef YMO_WSGI_TWORKER_H
#define YMO_WSGI_TWORKER_H
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pthread.h>
#include <ev.h>

#include "ymo_queue.h"
#include "ymo_wsgi_context.h"

/**---------------------------------
 *          Definitions
 *---------------------------------*/

/** Data structure which represents a single thread worker.
 */
struct ymo_wsgi_worker {
    ymo_queue_t      queue_in;
    ymo_queue_t      queue_out;
    ymo_queue_node_t pool_in[YMO_WSGI_WORKER_QUEUE_POOL_SIZE];
    ymo_queue_node_t pool_out[YMO_WSGI_WORKER_QUEUE_POOL_SIZE];

    /* Callbacks: */
    ev_prepare  prepare_watcher;
    ev_check    check_watcher;
    ev_idle     idle_watcher;

    /* Output data is signalled to the evloop using an async watcher: */
    ev_async         event_out;
    pthread_mutex_t  lock_out;

    /* Inbound data is signalled using a condition var: */
    pthread_mutex_t  lock_in;
    pthread_cond_t   ready_in;

    /* And the thread itself: */
    pthread_t  thread;
    size_t     id;
    int        running;
};

/**---------------------------------
 *          Functions
 *---------------------------------*/

/** Initialize a worker thread.
 *
 */
ymo_status_t ymo_wsgi_worker_init(
        struct ev_loop* loop,
        size_t thread_id,
        ymo_wsgi_worker_t* worker);


/** Start a worker thread.
 *
 */
ymo_status_t ymo_wsgi_worker_start(ymo_wsgi_worker_t* worker);


/** Lock the worker input queue.
 *
 */
static inline int ymo_wsgi_worker_lock_in(ymo_wsgi_worker_t* worker)
{
    return pthread_mutex_lock(&(worker->lock_in));
}


/** Unlock the worker input queue.
 */
static inline int ymo_wsgi_worker_unlock_in(ymo_wsgi_worker_t* worker)
{
    return pthread_mutex_unlock(&(worker->lock_in));
}


/** Lock the worker output queue.
 *
 */
static inline int ymo_wsgi_worker_lock_out(ymo_wsgi_worker_t* worker)
{
    return pthread_mutex_lock(&(worker->lock_out));
}


/** Unlock the worker output queue.
 */
static inline int ymo_wsgi_worker_unlock_out(ymo_wsgi_worker_t* worker)
{
    return pthread_mutex_unlock(&(worker->lock_out));
}


/** Add an exchange to the workers input queue.
 */
ymo_status_t ymo_wsgi_worker_add_exchange(
        ymo_wsgi_worker_t* worker, ymo_wsgi_exchange_t* exchange);


/** Append body data to the response on in the output queue.
 *
 */
ymo_status_t ymo_wsgi_worker_response_body_append(
        ymo_wsgi_exchange_t* exchange,
        ymo_bucket_t* body_item,
        int done);


/** Signals across thread boundaries to notify a worker that input data
 * is available for processing.
 *
 */
ymo_status_t ymo_wsgi_worker_notify(ymo_wsgi_worker_t* worker);


/** Join the worker thread.
 */
int ymo_wsgi_worker_join(ymo_wsgi_worker_t* worker);


#endif /* YMO_WSGI_TWORKER_H */


