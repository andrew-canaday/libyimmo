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

#include "ymo_log.h"
#include "ymo_alloc.h"
#include "ymo_wsgi_exchange.h"
#include "ymo_wsgi_session.h"

/* TODO: provide workaround when no atomic support! */
#ifndef __STDC_NO_ATOMICS__
#include <stdatomic.h>
#else
#error "stdatomic.h is required to build yimmo-wsgi"
#endif



/*---------------------------------*
 *           Functions:
 *---------------------------------*/

ymo_wsgi_exchange_t* ymo_wsgi_exchange_create(void)
{
    ymo_wsgi_exchange_t* exchange = YMO_NEW(ymo_wsgi_exchange_t);
    if( exchange ) {
        exchange->no_pool = 1;
    }
    return exchange;
}


void ymo_wsgi_exchange_init(
        ymo_wsgi_exchange_t* exchange,
        ymo_wsgi_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response
        )
{
    int no_pool = exchange->no_pool;
    memset(exchange, 0, sizeof(ymo_wsgi_exchange_t));
    exchange->session = session;
    exchange->request = request;
    exchange->response = response;
    exchange->no_pool = no_pool;
    atomic_init(&(exchange->refcnt), 1);

    /* The exchange holds a reference to the session: */
    ymo_wsgi_session_incref(session);
}


void ymo_wsgi_exchange_free(
        ymo_wsgi_session_t* session, ymo_wsgi_exchange_t* exchange)
{
    if( exchange->no_pool ) {
        YMO_DELETE(ymo_wsgi_exchange_t, exchange);
    } else {
        exchange->next = session->pool.head;
        session->pool.head = exchange;
    }
}


size_t ymo_wsgi_exchange_incref(ymo_wsgi_exchange_t* exchange)
{
    size_t refcnt = atomic_fetch_add(&(exchange->refcnt), 1) + 1;
    YMO_WSGI_TRACE(_c_l "Exchange %p refcnt: %u" _c_r, (void*)exchange, refcnt);
    return refcnt;
}


size_t ymo_wsgi_exchange_decref(ymo_wsgi_exchange_t* exchange)
{
    size_t refcnt = atomic_fetch_sub(&(exchange->refcnt),1) - 1;

    YMO_WSGI_TRACE(_c_l "Exchange %p refcnt: %u" _c_r, (void*)exchange, refcnt);

#if defined(YIMMO_WSGI_TRACE_REFCNT) && (YIMMO_WSGI_TRACE_REFCNT == 1)
    if( refcnt < 0 ) {
        ymo_log_error(_c_l "Negative refcnt for exchange %p: %u" _c_r, exchange, refcnt);
    }
#endif

    if( refcnt == 0 ) {
        /* Each exchange holds a reference to the owning session: */
        ymo_wsgi_session_decref(exchange->session);
        YMO_WSGI_TRACE(_c_l "Freeing exchange: %p" _c_r, (void*)exchange);
        ymo_wsgi_exchange_free(exchange->session, exchange);
    }
    return refcnt;
}

