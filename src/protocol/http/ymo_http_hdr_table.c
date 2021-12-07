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


#define _POSIX_C_SOURCE 200809L
#include "ymo_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_alloc.h"
#include "ymo_http_hdr_table.h"

#if defined(YMO_TRACE_HDR_TABLE) && YMO_TRACE_HDR_TABLE == 1
#define YMO_HDR_TABLE_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__);
#else
#define YMO_HDR_TABLE_TRACE(fmt, ...)
#endif /* YMO_TRACE_HDR_TABLE */

/* TODO: replace this with constant, ala YMO_ALLOC_LT_OVERRIDE */
const char* ymo_http_hdr_hash_override_method(void)
{
    return YMO_HTTP_HDR_HASH_OVERRIDE_METHOD;
}

/*---------------------------------------------------------------------------*
 * WEAK override:
 *--------------------------------------------------------------------------*/
#if defined(HAVE_FUNC_ATTRIBUTE_WEAK) \
    && (HAVE_FUNC_ATTRIBUTE_WEAK == 1) \
    && defined(YMO_HTTP_HDR_HASH_ALLOW_WEAK) \
    && YMO_HTTP_HDR_HASH_ALLOW_WEAK
__attribute__((weak)) ymo_http_hdr_id_t ymo_http_hdr_hash_init(void)
{
    return 5;
}

__attribute__((weak)) ymo_http_hdr_id_t ymo_http_hdr_hash_ch(
        ymo_http_hdr_id_t h, char c)
{
    return (h*283) + (c & 0xdf);
}

__attribute__((weak)) ymo_http_hdr_id_t ymo_http_hdr_hash(
        const char* str_in, size_t* len)
{
    const char* hdr_start = str_in;
    char c;
    ymo_http_hdr_id_t h = YMO_HTTP_HDR_HASH_INIT();
    while( (c = *str_in++) ) {
        h = YMO_HDR_HASH_CH(h,c);
    }

    if( len ) {
        *len = (size_t)(str_in - hdr_start)-1;
    }
    return h & YMO_HDR_TABLE_MASK;
}
#endif /* HAVE_FUNC_ATTRIBUTE_WEAK */

/* TODO: allow override for this too / optional strcmp, at least! */
#define YMO_HTTP_HDR_CMP(current, hdr, h_id) \
    current->h_id == h_id

ymo_http_hdr_table_t* ymo_http_hdr_table_create()
{
    ymo_http_hdr_table_t* table = YMO_NEW0(ymo_http_hdr_table_t);
    if( table ) {
        ymo_http_hdr_table_init(table);
    }
    return table;
}

void ymo_http_hdr_table_init(ymo_http_hdr_table_t* table)
{
#if YMO_HDR_TABLE_POOL_SIZE
    for( size_t i = 0; i < YMO_HDR_TABLE_POOL_SIZE-1; i++ ) {
        table->pool.items[i].next = &(table->pool.items[i+1]);
    }
    table->pool.head = &(table->pool.items[0]);
#endif /* YMO_HDR_TABLE_POOL_SIZE */
    return;
}

static inline ymo_http_hdr_table_node_t* ymo_http_hdr_table_node_create(ymo_http_hdr_table_t* table)
{
    ymo_http_hdr_table_node_t* node = NULL;
#if YMO_HDR_TABLE_POOL_SIZE
    if( table->pool.head ) {
        node = table->pool.head;
        table->pool.head = node->next;
    } else {
        node = YMO_NEW(ymo_http_hdr_table_node_t);
        if( node ) {
            node->flags |= YMO_HDR_FLAG_NOPOOL;
        }
    }
#else
    node = YMO_NEW(ymo_http_hdr_table_node_t);
#endif /* YMO_HDR_TABLE_POOL_SIZE */
    return node;
}

static void ymo_http_hdr_table_node_free(
        ymo_http_hdr_table_t* table, ymo_http_hdr_table_node_t* node)
{
    if( node->buffer ) {
        free(node->buffer);
        node->buffer = NULL;
    }

#if YMO_HDR_TABLE_POOL_SIZE
    if( node->flags & YMO_HDR_FLAG_NOPOOL ) {
        YMO_DELETE(ymo_http_hdr_table_node_t, node);
    } else {
        node->next = table->pool.head;
        table->pool.head = node;
    }
#else
    YMO_DELETE(ymo_http_hdr_table_node_t, node);
#endif /* YMO_HDR_TABLE_POOL_SIZE */
    return;
}

void ymo_http_hdr_table_free(ymo_http_hdr_table_t* table)
{
    ymo_http_hdr_table_clear(table);
    YMO_DELETE(ymo_http_hdr_table_t, table);
}

static ymo_http_hdr_id_t table_insert_first(
        ymo_http_hdr_table_t* table,
        ymo_http_hdr_id_t h_id,
        ymo_http_hdr_id_t index,
        size_t hdr_len,
        const char* hdr,
        const char* value)
{
    ymo_http_hdr_table_node_t* node = ymo_http_hdr_table_node_create(table);
    if( node ) {
        node->h_id = h_id;
        node->flags = YMO_HDR_FLAG_DEFAULT;
        node->hdr = hdr;
        node->hdr_len = hdr_len;
        node->value = value;
        node->next = table->bucket[index];
        node->buffer = NULL;
        table->bucket[index] = node;
    } else {
        errno = ENOMEM;
        h_id = 0;
    }

    return h_id;
}


ymo_http_hdr_id_t ymo_http_hdr_table_insert_precompute(
        ymo_http_hdr_table_t* table,
        ymo_http_hdr_id_t h_id,
        const char* hdr,
        size_t hdr_len,
        const char* value)
{
    ymo_http_hdr_id_t index = h_id % YMO_HDR_TABLE_BUCKET_SIZE;
    ymo_http_hdr_table_node_t* current = table->bucket[index];
    while( current ) {
        if( YMO_HTTP_HDR_CMP(current, hdr, h_id) ) {
            if( current->buffer ) {
                free(current->buffer);
            }

            current->hdr = hdr;
            current->value = value;
            current->hdr_len = hdr_len;
            return h_id;
        }
        current = current->next;
    }

    return table_insert_first(table, h_id, index, hdr_len, hdr, value);
}


ymo_http_hdr_id_t ymo_http_hdr_table_insert(
        ymo_http_hdr_table_t* table, const char* hdr, const char* value)
{
    size_t hdr_len;
    ymo_http_hdr_id_t h_id = YMO_HDR_HASH_FN(hdr, &hdr_len);
    return ymo_http_hdr_table_insert_precompute(
            table, h_id, hdr, hdr_len, value);
}


ymo_http_hdr_id_t ymo_http_hdr_table_add(
        ymo_http_hdr_table_t* table, const char* hdr, const char* value)
{
    size_t hdr_len;
    ymo_http_hdr_id_t h_id = YMO_HDR_HASH_FN(hdr, &hdr_len);
    ymo_http_hdr_id_t index = h_id % YMO_HDR_TABLE_BUCKET_SIZE;
    ymo_http_hdr_table_node_t* current = table->bucket[index];
    while( current ) {
        if( YMO_HTTP_HDR_CMP(current, hdr, h_id) ) {
#define YMO_HTTP_HDR_SET_COOKIE_HACK 1
#if defined(YMO_HTTP_HDR_SET_COOKIE_HACK) && YMO_HTTP_HDR_SET_COOKIE_HACK
            /* HACK HACK HACK: This will break if the hash function is
             * overridden! */
            if( current->h_id == HDR_ID_SET_COOKIE ) {
                return table_insert_first(
                        table, HDR_ID_SET_COOKIE, index,
                        hdr_len, hdr, value);
            }
#endif /* YMO_HTTP_HDR_SET_COOKIE_HACK */

            /* TODO: tidy this up! */
            size_t value_len = strlen(value);
            if( !current->buffer ) {
                size_t buff_size =
                    strlen(current->value) + value_len + sizeof(",");
                current->buffer = malloc(buff_size);
                if( !current->buffer ) {
                    goto add_nomem;
                }
                snprintf(current->buffer, buff_size,
                        "%s,%s", current->value, value);
            } else {
                size_t buff_len = strlen(current->buffer);
                size_t buff_size = buff_len + value_len + sizeof(",");
                current->buffer = realloc(current->buffer, buff_size);
                if( !current->buffer ) {
                    goto add_nomem;
                }
                char* buff_end = current->buffer + buff_len;
                (*buff_end++) = ',';
                strncpy(buff_end, value, value_len);
            }
            goto add_done;
        }
        current = current->next;
    }

    return table_insert_first(table, h_id, index, hdr_len, hdr, value);

add_done:
    return h_id;

add_nomem:
    errno = ENOMEM;
    return 0;
}

ymo_http_hdr_id_t ymo_http_hdr_table_add_precompute(
        ymo_http_hdr_table_t* table,
        ymo_http_hdr_id_t h_id,
        const char* hdr,
        size_t hdr_len,
        const char* value)
{
    ymo_http_hdr_id_t index = h_id % YMO_HDR_TABLE_BUCKET_SIZE;
    ymo_http_hdr_table_node_t* current = table->bucket[index];
    while( current ) {
        if( YMO_HTTP_HDR_CMP(current, hdr, h_id) ) {
            /* HACK HACK HACK HACK: tidy up all of this... */
            size_t value_len = strlen(value);
            if( !current->buffer ) {
                size_t buff_size =
                    strlen(current->value) + value_len + sizeof(",");
                current->buffer = malloc(buff_size);
                if( !current->buffer ) {
                    goto add_nomem;
                }
                snprintf(current->buffer, buff_size,
                        "%s,%s", current->value, value);
            } else {
                size_t buff_len = strlen(current->buffer);
                size_t buff_size = buff_len + value_len + sizeof(",");
                current->buffer = realloc(current->buffer, buff_size);
                if( !current->buffer ) {
                    goto add_nomem;
                }
                char* buff_end = current->buffer + buff_len;
                (*buff_end++) = ',';
                strncpy(buff_end, value, value_len);
            }
            goto add_done;
        }
        current = current->next;
    }

    return table_insert_first(table, h_id, index, hdr_len, hdr, value);

add_done:
    return h_id;

add_nomem:
    errno = ENOMEM;
    return 0;
}

const char* ymo_http_hdr_table_get(
        const ymo_http_hdr_table_t* table, const char* hdr)
{
    ymo_http_hdr_id_t h_id = YMO_HDR_HASH_FN(hdr, NULL);
    return ymo_http_hdr_table_get_id(table, h_id);
}

const char* ymo_http_hdr_table_get_id(
        const ymo_http_hdr_table_t* table, ymo_http_hdr_id_t h_id)
{
    const char* data = NULL;
    ymo_http_hdr_id_t index = h_id % YMO_HDR_TABLE_BUCKET_SIZE;
    ymo_http_hdr_table_node_t* current = table->bucket[index];
    while( current ) {
        /* TODO: compare strings to ensure it's not just hash collision. */
        if( YMO_HTTP_HDR_CMP(current, hdr, h_id) ) {
            if( !current->buffer ) {
                data = current->value;
            } else {
                data = current->buffer;
            }
            break;
        }
        current = current->next;
    }
    return data;
}

void ymo_http_hdr_table_clear(ymo_http_hdr_table_t* table)
{
    ymo_http_hdr_table_node_t* current = NULL;
    for( ymo_http_hdr_id_t index = 0;
         index < YMO_HDR_TABLE_BUCKET_SIZE; index++ ) {

        current = table->bucket[index];
        while( current ) {
            ymo_http_hdr_table_node_t* dnode = current;
            current = current->next;
            ymo_http_hdr_table_node_free(table, dnode);
        }
        table->bucket[index] = NULL;
    }

}


ymo_http_hdr_ptr_t ymo_http_hdr_table_next(
        ymo_http_hdr_table_t* table,
        ymo_http_hdr_ptr_t cur,
        const char** hdr,
        size_t* hdr_len,
        const char** value
        )
{
    /* If we've already started, jump right to finding: */
    if( cur ) {
        /* If we've got another in this bucket, use that! */
        if( cur->next ) {
            cur = cur->next;
            goto found_next;
        }

        /* Otherwise, we'll have to jump bucket-to-bucket: */
        ymo_http_hdr_id_t h_id = cur->h_id;
        size_t h_start = (h_id % YMO_HDR_TABLE_BUCKET_SIZE)+1;
        for( size_t i = h_start; i < YMO_HDR_TABLE_BUCKET_SIZE; i++ ) {
            cur = table->bucket[i];
            if( cur ) {
                goto found_next;
            }
        }

        return NULL;

found_next:
        {
            *hdr = cur->hdr;
            *hdr_len = cur->hdr_len;
            if( !cur->buffer ) {
                *value = cur->value;
            } else {
                *value = cur->buffer;
            }

            return cur;
        }
    } else {
        for( size_t i = 0; i < YMO_HDR_TABLE_BUCKET_SIZE; i++ ) {
            cur = table->bucket[i];
            if( cur ) {
                goto found_next;
            }
        }
    }

    /* If we make it here, the header table is empty. */
    return cur;
}




