/*=============================================================================
 *
 * Copyright (c) 2014 Andrew Canaday
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



#ifndef YIMMO_LIST_H
#define YIMMO_LIST_H

#include <stddef.h>
#include "yimmo.h"

/** Intrusive List
 * ================
 *
 * .. todo::
 *
 *    Check for ``typeof()`` availability.
 */

/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

typedef int (*ymo_list_cmp_fn)(void* l, void* r);

/**
 * Generic list datastructure.
 */
typedef struct ymo_list_head ymo_list_head_t;

/** Simple linked list head structure. */
struct ymo_list_head {
    struct ymo_list_head* next;
    struct ymo_list_head* prev;
};

/* HACK: Pointer arithmetic cast type.
 *
 * TODO:
 *  - check for void* arithmetic support
 *  - use ptrdiff_t?
 */
#define YMO_LIST_PTR_AR_TYPE const char*

/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

/** Insert an item into a list between ``prev`` and ``next`` (both of which
 * may be null).
 */
static inline void ymo_list_insert(
        ymo_list_head_t* prev,
        ymo_list_head_t* current,
        ymo_list_head_t* next
        )
{
    if( prev ) {
        prev->next = current;
    }

    current->prev = prev;
    current->next = next;

    if( next ) {
        next->prev = current;
    }
    return;
}


static inline const ymo_list_head_t* ymo_list_last(const ymo_list_head_t* current)
{
    if( current ) {
        while( current->next ) {
            current = current->next;
        }
    }
    return current;
}


/** Get the next item in a list. */
static inline const ymo_list_head_t* ymo_list_next(const ymo_list_head_t* head)
{
    return head->next;
}


/** Get the previous item in a list. */
static inline const ymo_list_head_t* ymo_list_prev(const ymo_list_head_t* head)
{
    return head->prev;
}


/**---------------------------------------------------------------
 * Type-generic Macros
 *---------------------------------------------------------------*/

#define YMO_LIST_NAME_DEFAULT yl_head

/** Insert a list into a structure, using a specific name.
 *
 * .. code-block:: c
 *    :caption: Example
 *
 *    typedef struct my_type {
 *        YMO_LIST_HEAD_M(head);
 *        char my_data[100];
 *    } my_type_t;
 *
 */
#define YMO_LIST_HEAD_M(name) \
        ymo_list_head_t name


/** YMO_LIST_HEAD_M using a generic name.
 *
 * (See :c:macro:`YMO_LIST_HEAD_M`)
 */
#define YMO_LIST_HEAD() \
        ymo_list_head_t YMO_LIST_NAME_DEFAULT

/** Initialize a list head object.
 *
 * .. code-block:: c
 *    :caption: Example
 *
 *    my_type_t my_obj;
 *    YMO_LIST_INIT_M(&my_obj, head);
 *
 */
#define YMO_LIST_INIT_M(p, m) \
        ((p)->m->prev = (p)->m->next = NULL)


/** YMO_LIST_INIT_M using a generic name.
 *
 * (See :c:macro:`YMO_LIST_INIT_M`)
 */
#define YMO_LIST_INIT(p) \
        YMO_LIST_INIT_M(p, YMO_LIST_NAME_DEFAULT)


/** Append an item to c list.
 *
 * .. code-block:: c
 *    :caption: Example
 *
 *    my_type_t c;
 *    my_type_t n;
 *
 *    YMO_LIST_INIT(&c, head);
 *    YMO_LIST_INIT(&n, head);
 *    YMO_LIST_APPEND_M(&c, &n, head);
 */
#define YMO_LIST_APPEND_M(c, n, m) \
        ymo_list_insert(&c->m, &n->m, NULL)


/** YMO_LIST_APPEND_M using c generic name.
 *
 * (See :c:macro:`YMO_LIST_APPEND_M`)
 */
#define YMO_LIST_APPEND(c, n) \
        YMO_LIST_APPEND_M(c, n, YMO_LIST_NAME_DEFAULT)


/** */
#define YMO_LIST_INSERT_M(p, c, n, m) \
        ymo_list_insert(p->m, c->m, n->m)


/** YMO_LIST_INSERT_M using a generic name.
 *
 * (See :c:macro:`YMO_LIST_INSERT_M`)
 */
#define YMO_LIST_INSERT(p, c, n) \
        YMO_LIST_INSERT_M(p, c, n, YMO_LIST_NAME_DEFAULT)


/** */
#define YMO_LIST_NEXT_M(p, t, m) \
        ((t*)(((YMO_LIST_PTR_AR_TYPE)ymo_list_next(&p->m)) - offsetof(t,m)))


/** YMO_LIST_NEXT_M using a generic name.
 *
 * (See :c:macro:`YMO_LIST_NEXT_M`)
 */
#define YMO_LIST_NEXT(p, t) \
        YMO_LIST_NEXT_M(p, t, YMO_LIST_NAME_DEFAULT)


/** */
#define YMO_LIST_PREV_M(p, t, m) \
        ((t*)(((YMO_LIST_PTR_AR_TYPE)ymo_list_prev(&p->m)) - offsetof(t,m)))


/** YMO_LIST_PREV_M using a generic name.
 *
 * (See :c:macro:`YMO_LIST_PREV_M`)
 */
#define YMO_LIST_PREV(p, t) \
        YMO_LIST_PREV_M(p, t, YMO_LIST_NAME_DEFAULT)


/** */
#define YMO_LIST_LAST_M(c, t, m) \
        ((t*)(((YMO_LIST_PTR_AR_TYPE)ymo_list_last(&c->m)) - offsetof(t,m)))


/** YMO_LIST_LAST_M using a generic name.
 *
 * (See :c:macro:`YMO_LIST_LAST_M`)
 */
#define YMO_LIST_LAST(c, t) \
        YMO_LIST_LAST_M(c, t, YMO_LIST_NAME_DEFAULT)


#endif /* YIMMO_LIST_H */

