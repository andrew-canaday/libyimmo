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



#ifndef YIMMO_QUEUE_H
#define YIMMO_QUEUE_H

#include "yimmo.h"

/** Queue
 * =======
 */

/*==================*
 *===-- Types: --===*
 *==================*/

typedef int (*ymo_queue_cmp_fn)(void* l, void* r);

/** .. c:type: ymo_queue_t
 *
 * Generic queue datastructure.
 */
typedef struct ymo_queue ymo_queue_t;


/** .. c:type: ymo_queue_node_t
 *
 * A queue node.
 */
typedef struct ymo_queue_node ymo_queue_node_t;

struct ymo_queue {
    ymo_queue_node_t* head;     /* First item in the linked queue of items */
    ymo_queue_node_t* tail;     /* Last item in the linked queue of items */
    size_t            no_items; /* Number of items in the queue */
    ymo_queue_node_t* pool;
    ymo_queue_node_t* pstack;
};


struct ymo_queue_node {
    ymo_queue_t*           queue;
    struct ymo_queue_node* next;
    struct ymo_queue_node* prev;
    void*                  data;
};


/*======================*
 *===-- Functions: --===*
 *======================*/
/** Allocate an initialize a new ymo queue. */
ymo_queue_t* ymo_queue_create(void);

/** Initialize a statically allocated ymo_queue */
static inline void ymo_queue_init(ymo_queue_t* queue)
{
    memset(queue, 0, sizeof(ymo_queue_t));
}

/** Create a new ymo queue with a node pool. */
ymo_queue_t* ymo_queue_create_pool(size_t n);

/** Initialize a statically allocated array of queue nodes to be used
 * as a pool for the given queue. */
void ymo_queue_pool_init(
        ymo_queue_t* queue,
        ymo_queue_node_t* pool,
        size_t n);

/** Free an ymo queue. */
void ymo_queue_free(ymo_queue_t* queue);

/** Append an item to the queue. */
ymo_status_t ymo_queue_append(ymo_queue_t* queue, void* item);

/** Append the contents of src to dst, clearing src: */
ymo_status_t ymo_queue_append_queue(ymo_queue_t* dst, ymo_queue_t* src);

/** Prepend an item to the queue. */
ymo_status_t ymo_queue_prepend(ymo_queue_t* queue, void* item);

/** Pop the front item off the queue. */
void* ymo_queue_popfront(ymo_queue_t* queue);

/** Peek the front item in the queue. */
void* ymo_queue_peekfront(ymo_queue_t* queue);

/** Pop the back item off the queue. */
void* ymo_queue_popback(ymo_queue_t* queue);

/** Peek the back item in the queue. */
void* ymo_queue_peekback(ymo_queue_t* queue);

/** Get the head node of the queue. */
const ymo_queue_node_t* ymo_queue_head(ymo_queue_t* queue);

/** Get the tail node of the queue. */
const ymo_queue_node_t* ymo_queue_tail(ymo_queue_t* queue);

/** Find an item using the given comparison function. */
void* ymo_queue_find(
        ymo_queue_t* queue, ymo_queue_cmp_fn cmp_fn, void* item);

/** Remove an item from the queue: */
ymo_status_t ymo_queue_remove(ymo_queue_t* queue, ymo_queue_node_t** node);

/** Return the number of items in the queue. */
size_t ymo_queue_size(ymo_queue_t* queue);

#endif /* YIMMO_QUEUE_H */



