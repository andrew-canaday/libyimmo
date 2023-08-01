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


#include "yimmo_config.h"
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

#if YMO_BUCKET_FROM_FILE == YM_BUCKET_MMAP
#include <sys/mman.h>
#endif /* ymo_bucket_from_file mmap */



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
        bucket->cleanup_cb = NULL;
#if YMO_BUCKET_FROM_FILE == YMO_BUCKET_SENDFILE
        bucket->fd = -1;
#endif /* YMO_BUCKET_SENDFILE */

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
        if( buf && buf_len ) {
            bucket->buf = YMO_ALLOC(buf_len);
            bucket->buf_len = buf_len;
            memcpy(bucket->buf, buf, buf_len);
        } else {
            bucket->buf = NULL;
            bucket->buf_len = 0;
        }
        bucket->data = bucket->buf;
        bucket->len = bucket->buf_len;
        bucket->bytes_sent = 0;
        bucket->next = (prev && prev->next) ? prev->next : next;
        bucket->cleanup_cb = NULL;
#if YMO_BUCKET_FROM_FILE == YMO_BUCKET_SENDFILE
        bucket->fd = -1;
#endif /* YMO_BUCKET_SENDFILE */

        if( prev ) {
            prev->next = bucket;
        }
    }
    return bucket;
}


#if YMO_BUCKET_FROM_FILE == YMO_BUCKET_FSTREAM
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
            YMO_FREE(buffer);
            return NULL;
        }

        return ymo_bucket_create(
                prev, next, buffer, fsize, buffer, fsize);
    }
    return NULL;
}


#endif /* YMO_BUCKET_FROM_FILE fstream */


#if YMO_BUCKET_FROM_FILE == YMO_BUCKET_MMAP
static void bucket_unmap(ymo_bucket_t* bucket)
{
    ymo_log_debug("Freeing mmap'd bucket: %p", (void*)bucket);
    int rc = munmap(bucket->data, bucket->len+1);

    if( rc < 0 ) {
        ymo_log_error("Error unmapping bucket: %s", strerror(errno));
    }
    return;
}


/* TODO: (lots of things)
 *
 * - use sendfile where available/appropriate.
 * - need lookup/counter to prevent the same file from being mmap'd when
 *   already resident
 * - if we're loading into mem anyway: leave it around and cache using
 *   some heuritstic
 */
ymo_bucket_t* ymo_bucket_from_file(
        ymo_bucket_t* restrict prev, ymo_bucket_t* restrict next,
        const char* filepath)
{
    int f_err = 0;

    /* Create bucket: */
    ymo_bucket_t* f_bucket = ymo_bucket_create(
            NULL, NULL,
            NULL, 0,
            NULL, 0);

    if( !f_bucket ) {
        return YMO_ERROR_PTR(ENOMEM);
    }

    /* Open file: */
    int fd = open(filepath, O_RDONLY);
    if( fd < 0 ) {
        f_err = errno;
        ymo_log_error("Unable to open filepath \"%s\": %s",
                filepath, strerror(f_err));
        goto bucket_file_fail;
    }

    /* Stat it: */
    struct stat f_info;
    if( fstat(fd, &f_info) < 0 ) {
        f_err = errno;
        ymo_log_error("Unable to stat filepath \"%s\": %s",
                filepath, strerror(f_err));
        goto bucket_file_fail;
    }

    /* Map into memory: */
    void* f_mem = mmap(0, f_info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if( f_mem != MAP_FAILED ) {
        ymo_log_debug("Created map of size %zu at %p from file %s",
                (size_t)f_info.st_size, f_mem, filepath);
        f_bucket->data = f_mem;
        f_bucket->len = f_info.st_size-1;
        f_bucket->dealloc = &bucket_unmap;
        return f_bucket;
    } else {
        f_err = errno;
        ymo_log_error("Unable to map file \"%s\" (%zu) into memory: %s",
                filepath, (size_t)f_info.st_size, strerror(f_err));
    }


bucket_file_fail:
    YMO_DELETE(ymo_bucket_t, f_bucket);
    return YMO_ERROR_PTR(f_err);
}


#endif /* YMO_BUCKET_FROM_FILE mmap */

#if YMO_BUCKET_FROM_FILE == YMO_BUCKET_SENDFILE
static void ymo_bucket_close_fd(ymo_bucket_t* bucket)
{
    if( bucket->fd >= 0 ) {
        int rc = 0;
        if( (rc = close(bucket->fd)) ) {
            ymo_log_warning("Failed to close file: %s", strerror(errno));
        }
        bucket->fd = -1;
    }
    return;
}


ymo_bucket_t* ymo_bucket_from_file(
        ymo_bucket_t* restrict prev, ymo_bucket_t* restrict next,
        const char* filepath)
{
    /* Create bucket: */
    ymo_bucket_t* f_bucket = ymo_bucket_create(
            NULL, NULL,
            NULL, 0,
            NULL, 0);

    if( !f_bucket ) {
        return YMO_ERROR_PTR(ENOMEM);
    }

    int f_err = 0;
    f_bucket->fd = open(filepath, O_RDONLY | O_NONBLOCK);
    if( f_bucket->fd >= 0 ) {
        struct stat f_info;
        if( fstat(f_bucket->fd, &f_info) < 0 ) {
            f_err = errno;
            goto bucket_sendfile_fail;
        }

        f_bucket->len = f_info.st_size;
        f_bucket->cleanup_cb = &ymo_bucket_close_fd;
        return f_bucket;
    }
    f_err = errno;

bucket_sendfile_fail:
    ymo_log_error("Unable to send file \"%s\": %s",
            filepath, strerror(f_err));
    YMO_DELETE(ymo_bucket_t, f_bucket);
    return YMO_ERROR_PTR(f_err);
}


#endif /* sendfile ymo_bucket_from_file */


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
    if( !bucket ) {
        return;
    }

    if( bucket->cleanup_cb ) {
        bucket->cleanup_cb(bucket);
    }

    YMO_FREE(bucket->buf);
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

