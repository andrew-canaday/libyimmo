/*=============================================================================
 * libyimmo: Lightweight socket server framework
 *
 * Copyright (c) 2014 Andrew Canaday
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




/** Buckets
 * =========
 *
 */

#ifndef YMO_BUCKET_H
#define YMO_BUCKET_H

#include "ymo_config.h"
#include <stddef.h>
#include "yimmo.h"

/**---------------------------------------------------------------
 * Macros
 *---------------------------------------------------------------*/

/** Bucket-type struct header.
 *
 * :param data: - Pointer to the payload data
 * :param len:  - Payload length
 * :param next: - Next item in chain
 */
#define YMO_BUCKET_HEAD(t) \
    const char* data; \
    size_t len; \
    t* next;

#if defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1)
YMO_ENUM8_TYPEDEF(ymo_bucket_code) {
    YMO_BUCKET_CTRL_CONTINUE,
    YMO_BUCKET_CTRL_CALL,
} YMO_ENUM8_AS(ymo_bucket_code_t);

/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/* TODO: this should probably take the bucket data, as well...
 * TODO: this isn't used...
 */
typedef ymo_status_t (*ymo_bucket_cb_t)(void* data);

typedef struct ymo_bucket_cb_info {
    ymo_bucket_cb_t  ctrl_cb;
    void*            ctrl_data;
} ymo_bucket_cb_info_t;
#endif /* defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1) */

/** Internal data structure used to buffer writes.
 * TODO: Add cleanup callback function?
 */
struct ymo_bucket {
    YMO_BUCKET_HEAD(ymo_bucket_t)
    char* buf;                      /* Optional pointer to managed memory */
    size_t              buf_len;    /* Length of the managed memory */
    size_t              bytes_sent; /* Total number of bytes sent */
    ymo_bucket_free_fn  dealloc;    /* Deallocation function */
#if defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1)
    ymo_bucket_code_t   ctrl_code;  /* Bucket out-of-band control code */
    void*               ctrl_data;  /* Optional out-of-band control data */
#endif /* defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1) */
};


#if defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1)
/* Set the special "code" flags on a bucket, for internal use. */
void ymo_bucket_set_ctrl_code(
        ymo_bucket_t* bucket, ymo_bucket_code_t code, void* data);
#endif /* defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1) */

#endif /* YMO_BUCKET_H */



