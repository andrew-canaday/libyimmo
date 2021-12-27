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

#define YMO_BUCKET_FSTREAM   0  /* TODO: switch to unbuffered IO */
#define YMO_BUCKET_MMAP      1
#define YMO_BUCKET_SENDFILE  2
#define YMO_BUCKET_FROM_FILE YMO_BUCKET_FSTREAM

#include "ymo_config.h"
#include <stddef.h>
#include "yimmo.h"


/**---------------------------------------------------------------
 * Data Structures
 *---------------------------------------------------------------*/

/** Internal data structure used to buffer writes. */
struct ymo_bucket {
    const char*         data;       /* Pointer to the payload data. */
    size_t              len;        /* Payload length */
    char*               buf;        /* Optional pointer to managed memory */
    size_t              buf_len;    /* Length of the managed memory */
    size_t              bytes_sent; /* Total number of bytes sent */
#if YMO_BUCKET_FROM_FILE == YMO_BUCKET_SENDFILE
    int                 fd;         /* TODO: HAVE_DECL_SENDFILE? */
#endif /* YMO_BUCKET_SENDFILE */
    ymo_bucket_t*       next;       /* Next bucket in the chain */
    ymo_bucket_free_fn  cleanup_cb; /* Cleanup callback */
};


#endif /* YMO_BUCKET_H */



