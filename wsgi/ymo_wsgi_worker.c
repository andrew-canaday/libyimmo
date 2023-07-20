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

/** # ymo_wsgi_worker.c
 * Threading, locking, and handoff between libev and python
 *
 *
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ev.h>
#include "ymo_log.h"

#include "yimmo_wsgi.h"
#include "ymo_wsgi_mod.h"
#include "ymo_wsgi_proc.h"
#include "ymo_wsgi_server.h"
#include "ymo_wsgi_worker.h"
#include "ymo_wsgi_session.h"
#include "ymo_wsgi_util.h"

/*---------------------------------*
 *          Prototypes:
 *---------------------------------*/
static void* ymo_wsgi_worker_main(void*);


/*---------------------------------*
 *           Functions:
 *---------------------------------*/
ymo_status_t ymo_wsgi_worker_add_exchange(
        ymo_wsgi_worker_t* worker, ymo_wsgi_exchange_t* exchange)
{
    /* Add one reference that the yimm_Context_t will end up owning: */
    WSGI_EXCHANGE_INCREF(exchange); /* +1 for worker */
    return ymo_queue_append(&worker->queue_in, exchange);
}


int ymo_wsgi_worker_notify(ymo_wsgi_worker_t* worker)
{
    return pthread_cond_signal(&(worker->ready_in));
}


static void ymo_wsgi_worker_reset(ymo_wsgi_worker_t* worker)
{
    return;
}


/*---------------------------------*
 *      Worker Initialization:
 *---------------------------------*/
ymo_status_t ymo_wsgi_worker_init(
        struct ev_loop* loop,
        size_t thread_id,
        ymo_wsgi_worker_t* worker
        )
{
    ymo_status_t status = YMO_OKAY;
    ymo_queue_init(&worker->queue_in);
    ymo_queue_pool_init(&worker->queue_in, worker->pool_in, YMO_WSGI_WORKER_QUEUE_POOL_SIZE);
    ymo_queue_init(&worker->queue_out);
    ymo_queue_pool_init(&worker->queue_out, worker->pool_out, YMO_WSGI_WORKER_QUEUE_POOL_SIZE);

    int init_fail = pthread_mutex_init(&(worker->lock_in), NULL);
    if( init_fail ) {
        status = init_fail;
        goto worker_init_bail;
    }

    init_fail = pthread_mutex_init(&(worker->lock_out), NULL);
    if( init_fail ) {
        status = init_fail;
        goto worker_init_bail;
    }

    init_fail = pthread_cond_init(&(worker->ready_in), NULL);
    if( init_fail ) {
        status = init_fail;
        goto worker_init_bail;
    }

    ev_async_init(&worker->event_out, ymo_wsgi_server_async);
    ev_prepare_init(&worker->prepare_watcher, ymo_wsgi_server_prepare);
    ev_check_init(&worker->check_watcher, ymo_wsgi_server_check);
    worker->event_out.data = worker;
    worker->prepare_watcher.data = worker;
    worker->check_watcher.data = worker;

    ev_async_start(loop, &worker->event_out);
    ev_prepare_start(loop, &worker->prepare_watcher);
    ev_check_start(loop, &worker->check_watcher);

#ifdef YMO_WSGI_USE_IDLE_WATCHER
    ev_idle_init(&worker->idle_watcher, ymo_wsgi_server_idle);
    ev_idle_start(loop, &worker->idle_watcher);
#endif /* YMO_WSGI_USE_IDLE_WATCHER */

    worker->id = thread_id;
    return YMO_OKAY;

worker_init_bail:
    ymo_wsgi_worker_reset(worker);
    return status;
}


ymo_status_t ymo_wsgi_worker_start(ymo_wsgi_worker_t* worker)
{
    ymo_status_t status = YMO_OKAY;
    worker->running = 1;
    /* TODO: the Python VM does some special stuff before spawning threads —
     * e.g. setting the stack size to ensure that a maximum recursion error
     * is thrown before the stack is fully consumed.
     *
     * This is a compelling reason to move to the module vs embedded approach.
     * For the time being, we'll just note it.
     */
    int thread_fail = pthread_create(&(worker->thread), NULL,
            ymo_wsgi_worker_main, worker);

    if( thread_fail ) {
        status = thread_fail;
        goto worker_start_bail;
    }

    thread_fail = pthread_detach(worker->thread);
    if( thread_fail ) {
        status = thread_fail;
        goto worker_start_bail;
    }
    return YMO_OKAY;

worker_start_bail:
    ymo_wsgi_worker_reset(worker);
    return status;
}


int ymo_wsgi_worker_join(ymo_wsgi_worker_t* worker)
{
    return pthread_join(worker->thread, NULL);
}


/*---------------------------------*
 *   Thread Worker Functions:
 *---------------------------------*/
ymo_status_t ymo_wsgi_worker_response_body_append(
        ymo_wsgi_exchange_t* exchange,
        ymo_bucket_t* body_item,
        int done
        )
{
    /* If the session's closed, stop appending: */
    if( ymo_wsgi_session_maybe_closed(exchange->session) && !done ) {
        ymo_bucket_free(body_item);
        return EPIPE;
    }

    ymo_wsgi_worker_t* worker = exchange->session->worker;
    Py_BEGIN_ALLOW_THREADS
    pthread_mutex_lock(&worker->lock_out);
    WSGI_EXCHANGE_INCREF(exchange); /* +1 for queue */
    YMO_WSGI_TRACE("Queuing body data for %p", (void*)exchange);
    if( exchange->body_data ) {
        ymo_bucket_append(exchange->body_data, body_item);
    } else {
        exchange->body_data = body_item;
    }
    if( done ) {
        YMO_WSGI_TRACE("Body data done for %p", (void*)exchange);
    }
    exchange->done = done;

    ymo_queue_append(&worker->queue_out, exchange);
    ev_async_send(EV_DEFAULT_ &worker->event_out);
    pthread_mutex_unlock(&worker->lock_out);
    Py_END_ALLOW_THREADS

    if( done ) {
        WSGI_EXCHANGE_DECREF(exchange); /* -1 for worker */
    }
    return YMO_OKAY;
}


static ymo_status_t ymo_wsgi_worker_issue_request(
        PyObject* pArgs,
        yimmo_context_t* ctx)
{
    int is_open = 1;
    ymo_status_t status = YMO_WOULDBLOCK;
    ymo_wsgi_exchange_t* exchange = ymo_wsgi_ctx_exchange(ctx);
    exchange->done = 0;

    /* Call the WSGI application function: */
    /* NOTE: May be use CallFunctionObjArgs? */
    PyObject* pClose = NULL;
    PyObject* r_val = NULL;
    r_val = PyObject_CallObject(pWsgiAppCallable, pArgs);
    Py_DECREF(pArgs);


    /* WSGI may return an iterable or invoke write(). Here we check the
     * return value: */
    if( !r_val || !PyIter_Check(r_val) ) {
        goto wsgi_result_cleanup;
    }

    ymo_bucket_t* body_item = NULL;
    PyObject* item;
    while( is_open && (item = PyIter_Next(r_val))) {
        if( PyBytes_Check(item) ) {
            Py_ssize_t content_len = PyBytes_GET_SIZE(item);
            char* content = PyBytes_AS_STRING(item);
            body_item = YMO_BUCKET_FROM_CPY(content, content_len);

            status =
                ymo_wsgi_worker_response_body_append(exchange, body_item, 0);
            if( status != YMO_OKAY ) {
                YMO_WSGI_TRACE("Whoops. Session closed with: %s",
                        strerror(status));
                is_open = 0;
            }
            body_item = NULL;
        }
        Py_DECREF(item);
    }

    if( PyErr_Occurred()) {
        /* TODO: handle this... */
        PyErr_Print();
    }

wsgi_response_cancel:
    /* Okay, send the last thing we've got: */
    status = ymo_wsgi_worker_response_body_append(exchange, body_item, 1);
    pClose = PyObject_GetAttr(r_val, pAttrClose);
    if( pClose ) {
        if( PyCallable_Check(pClose) ) {
            YMO_WSGI_TRACE("WSGI: invoking close() on %p", (void*)r_val);
            Py_CLEAR(r_val); /* Reuse */
#if PY_VERSION_HEX >= 0x03090000
            r_val = PyObject_CallNoArgs(pClose);
#else
            PyObject* emptyArgs = PyTuple_New(0);
            r_val = PyObject_Call(pClose, emptyArgs, NULL);
            Py_DECREF(emptyArgs);
#endif /* PY_VERSION_HEX */
        }
        Py_DECREF(pClose);
    }

wsgi_result_cleanup:
    Py_DECREF(YIMMO_CONTEXT_PY(ctx)); /* TODO: what if they use "write()"? */
    Py_XDECREF(r_val);
    return status;
}


#define ARG_ENVIRON 0
#define ARG_START_RESPONSE 1

static void* ymo_wsgi_worker_main(void* tdata)
{
    ymo_wsgi_worker_t* worker = tdata;
    PyGILState_STATE gstate;

    while( 1 ) {
        pthread_mutex_lock(&(worker->lock_in));

        /* If we have no pending requests. Yield the CPU and wait: */
        while( !ymo_queue_size(&worker->queue_in) ) {

            /* ...unless we're done working. Then just bail! */
            if( !worker->running ) {
                ymo_log_notice("Proc worker %i, thread %zu exiting.",
                        (int)getpid(), worker->id);
                pthread_mutex_unlock(&(worker->lock_in));
                return NULL;
            }
            pthread_cond_wait(&(worker->ready_in), &(worker->lock_in));
        }

        ymo_wsgi_exchange_t* exchange = ymo_queue_popfront(&worker->queue_in);
        pthread_mutex_unlock(&(worker->lock_in));

        /* TODO: unnecessary paranoia: */
        if( !exchange ) {
            ymo_log_warning("Got empty exchange: %p", (void*)exchange);
            continue;
        }

        gstate = PyGILState_Ensure();
        PyObject* pArgs = PyTuple_New(2);
        /* There seems to be less memory growth doing this than using
         * PyDict_Copy.
         */
        PyObject* pEnviron = PyDict_New();
        PyDict_Update(pEnviron, pCommonEnviron);

        errno = 0;
        yimmo_context_t* ctx = ymo_wsgi_ctx_update_environ(pEnviron, exchange);
        if( !ctx ) {
            int r_val = errno; errno = 0;
            Py_DECREF(pArgs);
            Py_DECREF(pEnviron);
            ymo_log_debug("Failed to create Context for exchange %p: %s",
                    (void*)exchange, strerror(r_val));
            WSGI_EXCHANGE_DECREF(exchange); /* -1 for worker */
            goto worker_gil_release;
        }

        /* Set up args: */
        PyTuple_SetItem(pArgs, ARG_ENVIRON, pEnviron);
        PyTuple_SetItem(pArgs, ARG_START_RESPONSE,
                PyObject_GetAttr(YIMMO_CONTEXT_PY(ctx), pAttrStartResponse));
        ymo_wsgi_worker_issue_request(pArgs, ctx);
worker_gil_release:
        PyGILState_Release(gstate);
    }

    return NULL;
}

