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


#include "ymo_config.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if HAVE_DECL_SENDFILE
/* TODO: have most of these already: */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#endif /* HAVE_DECL_SENDFILE */

#include "yimmo.h"
#include "ymo_util.h"
#include "ymo_log.h"
#include "ymo_net.h"
#include "ymo_conn.h"
#include "ymo_alloc.h"

#define YMO_TRACE_NET 0
#if defined(YMO_TRACE_NET) && YMO_TRACE_NET == 1
#define YMO_NET_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__);
#else
#define YMO_NET_TRACE(fmt, ...)
#endif /* YMO_TRACE_NET */


#if YMO_BUCKET_FROM_FILE == YMO_BUCKET_SENDFILE
#define YMO_NET_SENDFILE_MAX 1024
static ymo_status_t ymo_net_bucket_sendfile(int fd, ymo_bucket_t** head_p);
#endif /* YMO_BUCKET_FROM_FILE */


ymo_status_t ymo_net_send_buckets(int fd, ymo_bucket_t** head_p)
{
    size_t i;
    size_t to_send;
    ymo_bucket_t* current;

do_send:
    i = 0;
    to_send = 0;
    current = *head_p;
#if YMO_BUCKET_FROM_FILE == YMO_BUCKET_SENDFILE
    /* HACK: for now, we just check the FIRST bucket in the chain for an
     *       fd (see note below on how this works).
     */
    if( current && current->fd >= 0 ) {
        return ymo_net_bucket_sendfile(fd, head_p);
    }
#endif /* YMO_BUCKET_SENDFILE */

    struct msghdr out_msg;
    struct iovec out_vec[YMO_BUCKET_MAX_IOVEC];
    memset(&out_msg, 0, sizeof(struct msghdr));
    memset(&out_vec, 0, sizeof(struct iovec)*YMO_BUCKET_MAX_IOVEC);
    out_msg.msg_iov = (struct iovec*)&out_vec;

    /* Add all our buckets to the iovec: */
    while( current && i < YMO_BUCKET_MAX_IOVEC )
    {
#if YMO_BUCKET_FROM_FILE == YMO_BUCKET_SENDFILE
        /* HACK: *just for now*, we do sendmsg/sendfile separately (rather than
         *       user headers/footers)...
         *
         *       So, we sendfile for the first bucket in a chain and break
         *       on a bucket mid-chain (so it'll be first on next
         *       invocation).
         */
        if( current->fd >= 0 ) {
            break;
        }
#endif /* YMO_BUCKET_SENDFILE */

        /* Skip empty buckets: */
        if( current->len ) {
            out_vec[i].iov_base = (void*)(current->data + current->bytes_sent);
            out_vec[i].iov_len = current->len - current->bytes_sent;
            to_send += out_vec[i].iov_len;
            ++i;
        }

        current = current->next;
    }

    out_msg.msg_iovlen = i;

    /* Perform the send: */
    ssize_t bytes_sent = 0;
    if( to_send > 0 ) {
        YMO_NET_TRACE("Sending %lu bytes in %lu buckets", to_send, i);
        bytes_sent = sendmsg(fd, &out_msg, YMO_SEND_FLAGS);
        YMO_NET_TRACE("%i: Sent %lu bytes", fd, bytes_sent);
    } else {
        YMO_NET_TRACE("%i: No bytes sent; Zero iovecs", fd);
        return EBADMSG;
    }

    /* Bail on send error: */
    if( bytes_sent < 0 ) {
        ymo_log_debug("Failed to send to fd: %i (%s)", fd, strerror(errno));
        return errno;
    }

    /* Do bucket accounting, pruning off sent buckets: */
    current = *head_p;
    ymo_bucket_t* next = current;
    i = 0;
    while( current && bytes_sent > 0 ) {
        ssize_t remain = current->len - current->bytes_sent;

        /* Incomplete bucket send: */
        if( bytes_sent < remain ) {
            current->bytes_sent += bytes_sent;
            YMO_NET_TRACE("%i: bucket[%lu]: total bytes sent: %lu/%lu",
                    fd, i, remain, current->len);
            break;
        }
        /* Complete bucket send: */
        else {
            current->bytes_sent = current->len;
            YMO_NET_TRACE("%i: bucket[%lu]: total bytes sent: %lu/%lu",
                    fd, i, remain, current->len);
            bytes_sent -= remain;

            next = current->next;
            ymo_bucket_free(current);
            current = next;
        }
        ++i;
    }

    *head_p = current;
    if( current && current->len ) {
        YMO_NET_TRACE("%i: %lu/%lu bytes remain in next bucket",
                fd, (current->len - current->bytes_sent), current->len);
        goto do_send;
    } else {
        YMO_NET_TRACE("%i: Head is NULL. No more buckets to send", fd);
        return YMO_OKAY;
    }
}


#if YMO_BUCKET_FROM_FILE == YMO_BUCKET_SENDFILE
static ymo_status_t ymo_net_bucket_sendfile(int fd, ymo_bucket_t** head_p)
{
    ymo_bucket_t* f_bucket = *head_p;

    off_t len_remain = (f_bucket->len - f_bucket->bytes_sent);
    off_t send_len = YMO_MIN(len_remain,YMO_NET_SENDFILE_MAX);
    off_t len = send_len;
    errno = 0;
    int rc = sendfile(
            f_bucket->fd, fd,
            f_bucket->bytes_sent, &len,
            NULL, 0);

    if( rc == -1 ) {
        int s_err = errno;
        ymo_log_debug("Failed to send fd %i to socket: %i (%s)",
                f_bucket->fd, fd, strerror(s_err));
        return s_err;
    }

    if( !len ) {
        len = send_len;
    }

    if( len < len_remain ) {
        YMO_NET_TRACE("Sent %zu bytes to %i", (size_t)len, fd);
        f_bucket->bytes_sent += len;
        return YMO_WOULDBLOCK;
    }

    YMO_NET_TRACE("Sent %zu bytes to %i. File sent!", (size_t)len, fd);
    ymo_bucket_t* next = f_bucket->next;
    ymo_bucket_free(f_bucket);

    *head_p = next;

    if( next ) {
        YMO_NET_TRACE("%i: %lu/%lu bytes remain in next bucket",
                fd, (next->len - next->bytes_sent), next->len);
        return YMO_WOULDBLOCK;
    }
    return YMO_OKAY;
}


#endif /* YMO_BUCKET_SENDFILE */

