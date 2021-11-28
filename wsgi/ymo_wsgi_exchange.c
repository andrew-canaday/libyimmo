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
ymo_wsgi_exchange_t* ymo_wsgi_exchange_create(ymo_wsgi_session_t* session)
{
    ymo_wsgi_exchange_t* exchange;
    if( session->pool.head ) {
        exchange = session->pool.head;
        session->pool.head = exchange->next;
        memset(exchange, 0, sizeof(ymo_wsgi_exchange_t));
        exchange->session = session;
    } else {
        exchange = YMO_NEW0(ymo_wsgi_exchange_t);
        if( exchange ) {
            exchange->no_pool = 1;
            exchange->session = session;
        }
    }
    return exchange;
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
#if YMO_WSGI_EXCHANGE_ATOMIC
    size_t refcnt = atomic_fetch_add(&(exchange->refcnt), 1) + 1;
#else /* !YMO_WSGI_EXCHANGE_ATOMIC */
    size_t refcnt = ++exchange->refcnt;
#endif /* YMO_WSGI_EXCHANGE_ATOMIC */
    YMO_WSGI_TRACE("Exchange %p refcnt: %zu", (void*)exchange, refcnt);
    return refcnt;
}

size_t ymo_wsgi_exchange_decref(ymo_wsgi_exchange_t* exchange)
{
#if YMO_WSGI_EXCHANGE_ATOMIC
    size_t refcnt = atomic_fetch_sub(&(exchange->refcnt),1) - 1;
#else /* !YMO_WSGI_EXCHANGE_ATOMIC */
    size_t refcnt = --exchange->refcnt;
#endif /* YMO_WSGI_EXCHANGE_ATOMIC */

    YMO_WSGI_TRACE("Exchange %p refcnt: %zu", (void*)exchange, refcnt);
    if( refcnt == 0 ) {
        /* Each exchange holds a reference to the owning session: */
        ymo_wsgi_session_decref(exchange->session);
        YMO_WSGI_TRACE("Deleting exchange: %p", (void*)exchange);
        ymo_wsgi_exchange_free(exchange->session, exchange);
    }
    return refcnt;
}

