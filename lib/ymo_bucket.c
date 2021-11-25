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


#include "ymo_config.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_bucket.h"
#include "ymo_alloc.h"

ymo_bucket_t* ymo_bucket_create(
        ymo_bucket_t* restrict prev, ymo_bucket_t* restrict next,
        char* buf, size_t buf_len,
        const char* data, size_t len)
{
    ymo_bucket_t* bucket = YMO_NEW(ymo_bucket_t);
    if( bucket ) {
        bucket->buf = buf;
        bucket->buf_len = buf_len;
        bucket->data = data;
        bucket->len = len;
        bucket->bytes_sent = 0;
        bucket->next = (prev && prev->next) ? prev->next : next;
#if defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1)
        bucket->ctrl_code = YMO_BUCKET_CTRL_CONTINUE;
        bucket->ctrl_data = NULL;
#endif /* defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1) */

        if( prev ) {
            prev->next = bucket;
        }
    }
    return bucket;
}

ymo_bucket_t* ymo_bucket_create_cpy(
        ymo_bucket_t* restrict prev, ymo_bucket_t* restrict next,
        const char* buf, size_t buf_len)
{
    ymo_bucket_t* bucket = YMO_NEW(ymo_bucket_t);
    if( bucket ) {
        bucket->buf = YMO_ALLOC(buf_len);
        bucket->buf_len = buf_len;
        memcpy(bucket->buf, buf, buf_len);
        bucket->data = bucket->buf;
        bucket->len = bucket->buf_len;
        bucket->bytes_sent = 0;
        bucket->next = (prev && prev->next) ? prev->next : next;

        if( prev ) {
            prev->next = bucket;
        }
    }
    return bucket;
}

/* TODO: this should use flags + sendfile, where available */
ymo_bucket_t* ymo_bucket_from_file(
        ymo_bucket_t* restrict prev, ymo_bucket_t* restrict next,
        const char* filepath)
{
    long fsize = 0;
    char* buffer = NULL;
    FILE* in_file = fopen(filepath, "r");
    if( in_file ) {
        fseek(in_file, 0, SEEK_END);
        fsize = ftell(in_file);
        if( fsize < 0 ) {
            return NULL;
        }

        rewind(in_file);

        buffer = YMO_ALLOC(fsize);
        if( !buffer ) {
            errno = ENOMEM;
            return NULL;
        }

        size_t no_read = fread(buffer, 1, fsize, in_file);
        fclose(in_file);

        if( no_read != (unsigned long)fsize ) {
            errno = EFBIG;
            YMO_FREE(fsize, buffer);
            return NULL;
        }

        return ymo_bucket_create(
                prev, next, buffer, fsize, buffer, fsize);
    }
    return NULL;
}

#if defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1)
void ymo_bucket_set_ctrl_code(
        ymo_bucket_t* bucket, ymo_bucket_code_t code, void* data)
{
    bucket->ctrl_code = code;
    bucket->ctrl_data = data;
    return;
}
#endif /* defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1) */

size_t ymo_bucket_len_all(ymo_bucket_t* p)
{
    size_t len = 0;
    while( p ) {
        len += p->len;
        p = p->next;
    }
    return len;
}

ymo_bucket_t* ymo_bucket_tail(ymo_bucket_t* p)
{
    while( p && p->next ) {
        p = p->next;
    }
    return p;
}

ymo_bucket_t* ymo_bucket_append(
        ymo_bucket_t* restrict dst, ymo_bucket_t* restrict src)
{
    if( dst ) {
        ymo_bucket_t* tail = ymo_bucket_tail(dst);
        tail->next = src;
    }
    return ymo_bucket_tail(src);
}

void ymo_bucket_free(ymo_bucket_t* bucket)
{
    YMO_FREE(bucket->buf_len, bucket->buf);
    YMO_DELETE(ymo_bucket_t, bucket);
    return;
}

void ymo_bucket_free_all(ymo_bucket_t* bucket)
{
    while( bucket )
    {
        ymo_bucket_t* tmp = bucket;
        bucket = bucket->next;
        ymo_bucket_free(tmp);
    }
}



