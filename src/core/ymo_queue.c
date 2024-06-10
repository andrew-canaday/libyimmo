/*=============================================================================
 *
 *  Copyright (c) 2014 Andrew Canaday
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


#include <stdlib.h>
#include <string.h>
#include "ymo_alloc.h"
#include "ymo_queue.h"


static void delete_node(ymo_queue_node_t* node)
{
    if( node->queue ) {
        ymo_queue_t* queue = node->queue;
        if( queue->pstack ) {
            node->prev = NULL;
            node->next = queue->pstack;
            queue->pstack = node;
        } else {
            node->next = node->prev = NULL;
            queue->pstack = node;
        }
    } else {
        YMO_DELETE(ymo_queue_node_t, node);
    }
    return;
}


static inline ymo_queue_node_t* create_node(ymo_queue_t* queue)
{
    ymo_queue_node_t* node = queue->pstack;
    if( !node ) {
        node = YMO_NEW(ymo_queue_node_t);
        node->queue = NULL;
        return node;
    }

    /* If there's something in the pool, sweet: scoop it up! */
    queue->pstack = queue->pstack->next;
    node->next = node->prev = NULL;
    return node;
}


ymo_queue_t* ymo_queue_create(void)
{
    ymo_queue_t* queue = YMO_NEW0(ymo_queue_t);
    if( !queue ) {
        return NULL;
    }

    return queue;
}


ymo_queue_t* ymo_queue_create_pool(size_t n)
{
    ymo_queue_t* queue = YMO_NEW0(ymo_queue_t);
    if( !queue ) {
        return NULL;
    }

    ymo_queue_node_t* pool = calloc(n, sizeof(ymo_queue_node_t));
    if( !pool ) {
        YMO_DELETE(ymo_queue_t, queue);
        return NULL;
    }

    ymo_queue_pool_init(queue, pool, n);
    return queue;
}


void ymo_queue_pool_init(
        ymo_queue_t* queue,
        ymo_queue_node_t* pool,
        size_t n)
{
    size_t last = n-1;
    pool[0].prev = NULL;
    pool[0].next = &pool[1];
    pool[0].queue = queue;
    pool[last].next = NULL;
    pool[last].prev = &pool[last-1];
    pool[last].queue = queue;

    for( size_t i = 1; i < last; i++ ) {
        pool[i].prev = &pool[i-1];
        pool[i].next = &pool[i+1];
        pool[i].queue = queue;
    }

    queue->pool = pool;
    queue->pstack = pool;
    return;
}


void ymo_queue_free(ymo_queue_t* queue)
{
    ymo_queue_node_t* current = queue->head;
    while( current ) {
        ymo_queue_node_t* next = current->next;
        delete_node(current);
        current = next;
    }
    if( queue->pool ) {
        free(queue->pool);
    }
    YMO_DELETE(ymo_queue_t, queue);
}


ymo_status_t ymo_queue_append(ymo_queue_t* queue, void* item)
{
    ymo_queue_node_t* node = create_node(queue);
    if( !node ) {
        return ENOMEM;
    }

    node->data = item;
    node->next = NULL;
    node->prev = queue->tail;

    if( queue->tail ) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }

    queue->tail = node;
    ++queue->no_items;
    return YMO_OKAY;
}


ymo_status_t ymo_queue_append_queue(ymo_queue_t* dst, ymo_queue_t* src)
{
    dst->tail->next = src->head;
    dst->tail = src->tail;
    dst->no_items += src->no_items;
    src->head = NULL;
    src->no_items = 0;
    return YMO_OKAY;
}


ymo_status_t ymo_queue_prepend(ymo_queue_t* queue, void* item)
{
    ymo_queue_node_t* node = create_node(queue);
    if( !node ) {
        return ENOMEM;
    }

    node->data = item;
    node->next = queue->head;
    node->prev = NULL;

    if( queue->head ) {
        queue->head->prev = node;
    } else {
        queue->tail = node;
    }

    queue->head = node;
    ++queue->no_items;
    return YMO_OKAY;
}


void* ymo_queue_popfront(ymo_queue_t* queue)
{
    void* data = NULL;
    ymo_queue_node_t* node = queue->head;
    if( node ) {
        queue->head = node->next;

        if( node == queue->tail ) {
            queue->tail = NULL;
        }

        if( queue->head ) {
            queue->head->prev = NULL;
        }
        --queue->no_items;
        data = node->data;
        delete_node(node);
    }

    return data;
}


void* ymo_queue_popback(ymo_queue_t* queue)
{
    void* data = NULL;
    ymo_queue_node_t* node = queue->tail;
    if( node ) {
        queue->tail = node->prev;

        if( node == queue->head ) {
            queue->head = NULL;
        }

        if( queue->tail ) {
            queue->tail->next = NULL;
        }

        --queue->no_items;
        data = node->data;
        delete_node(node);
    }
    return data;
}


void* ymo_queue_peekfront(ymo_queue_t* queue)
{
    if( queue->head ) {
        return queue->head->data;
    } else {
        return NULL;
    }
}


void* ymo_queue_peekback(ymo_queue_t* queue)
{
    if( queue->tail ) {
        return queue->tail->data;
    } else {
        return NULL;
    }
}


void* ymo_queue_find(
        ymo_queue_t* queue, ymo_queue_cmp_fn cmp_fn, void* item)
{
    ymo_queue_node_t* current;
    for( current = queue->head; current; current = current->next )
    {
        if( current->data && (cmp_fn(current->data,item) == 0) ) {
            return current->data;
        }
    }
    return NULL;
}


const ymo_queue_node_t* ymo_queue_head(ymo_queue_t* queue)
{
    return queue->head;
}


const ymo_queue_node_t* ymo_queue_tail(ymo_queue_t* queue)
{
    return queue->tail;
}


/** Remove an item from the queue: */
ymo_status_t ymo_queue_remove(ymo_queue_t* queue, ymo_queue_node_t** node_ptr)
{
    ymo_queue_node_t* node = *node_ptr;
    if( !node ) {
        return EINVAL;
    }

    ymo_queue_node_t* item_next = node->next;
    ymo_queue_node_t* item_prev = node->prev;

    /* if next and previus are the same, and this isn't the head of the queue
     * it's not a member of the queue!: */
    if( item_prev == item_next && node != queue->head ) {
        return EINVAL;
    }

    if( item_prev ) {
        item_prev->next = item_next;
    }

    if( item_next ) {
        item_next->prev = item_prev;
    }

    if( node == queue->head ) {
        queue->head = item_next;
    }

    if( node == queue->tail ) {
        queue->tail = item_prev;
    }

    --queue->no_items;
    delete_node(node);
    *(node_ptr) = NULL;
    return YMO_OKAY;
}


size_t ymo_queue_size(ymo_queue_t* queue)
{
    return queue->no_items;
}

