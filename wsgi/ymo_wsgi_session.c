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

/** # ymo_wsgi_session.c
 * Types and functions to for individual WSGI sessions and their exchanges.
 *
 *
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pthread.h>

#define YMO_WSGI_SESSION_INTERNALS
#include "ymo_alloc.h"
#include "ymo_log.h"
#include "ymo_wsgi_session.h"
#include "ymo_wsgi_proc.h"
#include "ymo_wsgi_exchange.h"

/* TODO: provide workaround when no atomic support! */
#ifndef __STDC_NO_ATOMICS__
#include <stdatomic.h>
#else
#error "stdatomic.h is required to build yimmo-wsgi"
#endif



/* Prototypes: */
static void ymo_wsgi_session_free(ymo_wsgi_session_t* session);

/*---------------------------------*
 *           Functions:
 *---------------------------------*/
ymo_status_t ymo_wsgi_session_init(void* server_data, ymo_http_session_t* http_session)
{
    static size_t session_id = 1;
    ymo_wsgi_proc_t* w_proc = server_data;

    ymo_wsgi_session_t* session = YMO_NEW0(ymo_wsgi_session_t);
    if( session ) {
        session->id = session_id++;
        atomic_init(&(session->refcnt), 1);
        atomic_init(&(session->closed), 0);
        session->worker = ymo_wsgi_proc_assign_worker(w_proc, session->id);
        pthread_mutex_init(&session->lock, NULL);

        for( size_t i = 0; i < YMO_WSGI_EXCHANGE_POOL_SIZE-1; i++ ) {
            session->pool.items[i].next = &(session->pool.items[i+1]);
        }
        session->pool.head = &(session->pool.items[0]);

        ymo_http_session_set_userdata(http_session, session);
    } else {
        ymo_log_warning("Failed to create WSGI session for http: %p", (void*)http_session);
        return ENOMEM;
    }

    return YMO_OKAY;
}


static void ymo_wsgi_session_free(ymo_wsgi_session_t* session)
{
    YMO_WSGI_TRACE("Freeing ymo_wsgi_session: %p", (void*)session);
    YMO_DELETE(ymo_wsgi_session_t, session);
}


void ymo_wsgi_session_cleanup(
        void* server_data, ymo_http_session_t* http_session, void* data)
{
    YMO_WSGI_TRACE("Session cleanup for %p", data);
    ymo_wsgi_session_t* session = data;
    if( !session ) {
        return;
    }

    ymo_wsgi_session_close(session);
    ymo_wsgi_session_lock(session);
    size_t no_ex = 0;
    while( session->head ) {
        no_ex++;
        session->head->done = 1;
        ymo_wsgi_exchange_t* next = session->head->next;
        ymo_wsgi_exchange_decref(session->head);
        session->head = next;
    }
    session->head = NULL;
    YMO_WSGI_TRACE("Session cleanup with %zu exchanges present.", no_ex);
    ymo_wsgi_session_unlock(session);
    ymo_wsgi_session_decref(session);
    return;
}


ymo_wsgi_worker_t* ymo_wsgi_session_worker(ymo_wsgi_session_t* session)
{
    YMO_WSGI_TRACE("Worker for session %p: %zu", (void*)session, session->worker);
    return session->worker;
}


int ymo_wsgi_session_lock(ymo_wsgi_session_t* session)
{
    return pthread_mutex_lock(&(session->lock));
}


int ymo_wsgi_session_trylock(ymo_wsgi_session_t* session)
{
    if( ymo_wsgi_session_is_closed(session) ) {
        return EINVAL;
    }
    return pthread_mutex_trylock(&(session->lock));
}


int ymo_wsgi_session_unlock(ymo_wsgi_session_t* session)
{
    return pthread_mutex_unlock(&(session->lock));
}


size_t ymo_wsgi_session_incref(ymo_wsgi_session_t* session)
{
    size_t refcnt = atomic_fetch_add(&(session->refcnt), 1) + 1;
    YMO_WSGI_TRACE("WSGI session %p refcnt: %zu", (void*)session, refcnt);
    return refcnt;
}


size_t ymo_wsgi_session_decref(ymo_wsgi_session_t* session)
{
    size_t refcnt = atomic_fetch_sub(&(session->refcnt), 1) - 1;
    YMO_WSGI_TRACE("WSGI session %p refcnt: %zu", (void*)session, refcnt);
    if( refcnt == 0 ) {
        ymo_wsgi_session_free(session);
    }
    return refcnt;
}


void ymo_wsgi_session_close(ymo_wsgi_session_t* session)
{
    atomic_fetch_add(&(session->closed), 1);
    return;
}


int ymo_wsgi_session_is_closed(ymo_wsgi_session_t* session)
{
    int closed = atomic_load(&(session->closed));
    return closed;
}


/*=====================================================
 * Exchange Functions:
 *-----------------------------------------------------*/
ymo_wsgi_exchange_t* ymo_wsgi_session_create_exchange(
        ymo_wsgi_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response
        )
{
    ymo_wsgi_exchange_t* exchange = ymo_wsgi_exchange_create(session);
    if( !exchange ) {
        return NULL;
    }

#if YMO_WSGI_EXCHANGE_ATOMIC
    atomic_init(&(exchange->refcnt), 1);
#else /* !YMO_WSGI_EXCHANGE_ATOMIC */
    exchange->refcnt = 1;
#endif /* YMO_WSGI_EXCHANGE_ATOMIC */

    /* The exchange holds a reference to the session: */
    ymo_wsgi_session_incref(session);
    exchange->request = request;
    exchange->response = response;

    if( session->head ) {
        ymo_wsgi_exchange_t* last = session->head;
        while( last->next ) { last = last->next; }
        last->next = exchange;
    } else {
        session->head = exchange;
    }

    return exchange;
}


void ymo_wsgi_session_exchange_done(ymo_wsgi_session_t* session)
{
    if( !session->head ) {
        return;
    }

    ymo_wsgi_exchange_t* next = session->head->next;
    ymo_wsgi_exchange_decref(session->head);
    session->head = next;
}

