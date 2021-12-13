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

#include "yimmo.h"
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

ymo_status_t ymo_net_send_buckets(int fd, ymo_bucket_t** head_p)
{
    size_t i = 0;
    size_t to_send = 0;
    ymo_bucket_t* current = *head_p;

    struct msghdr out_msg;
    struct iovec out_vec[YMO_BUCKET_MAX_IOVEC];
    memset(&out_msg, 0, sizeof(struct msghdr));
    memset(&out_vec, 0, sizeof(struct iovec)*YMO_BUCKET_MAX_IOVEC);
    out_msg.msg_iov = (struct iovec*)&out_vec;

    /* Add all our buckets to the iovec: */
    while( current && i < YMO_BUCKET_MAX_IOVEC )
    {
        out_vec[i].iov_base = (void*)(current->data + current->bytes_sent);
        out_vec[i].iov_len = current->len - current->bytes_sent;
        to_send += out_vec[i].iov_len;
        ++i;

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

#if defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1)
            /* TODO: should this get removed? */
            switch( current->ctrl_code ) {
                case YMO_BUCKET_CTRL_CONTINUE:
                    break;
                case YMO_BUCKET_CTRL_CALL:
                {
                    ymo_bucket_cb_info_t* bucket_cb = (ymo_bucket_cb_info_t*)current->ctrl_data;
                    if( bucket_cb ) {
                        ymo_status_t cb_status = (*bucket_cb->ctrl_cb)(bucket_cb->ctrl_data);
                        YMO_DELETE(ymo_bucket_cb_info_t, bucket_cb);
                        if( cb_status != YMO_OKAY ) {
                            ymo_log_error("Bucket ctrl CALL for fd: %i", fd);
                            // TODO: goto err_bail
                            return cb_status;
                        }
                    } else {
                        // TODO: goto err_bail
                        return EINVAL;
                    }
                }
                break;
            }
#endif /* defined (YMO_BUCKET_CTRL_ENABLED) && (YMO_BUCKET_CTRL_ENABLED == 1) */

            next = current->next;
            ymo_bucket_free(current);
            current = next;
        }
        ++i;
    }

    *head_p = current;
    if( current ) {
        YMO_NET_TRACE("%i: %lu/%lu bytes remain in next bucket",
                fd, (current->len - current->bytes_sent), current->len);
        YMO_NET_TRACE("More buckets to send for %i", fd);
        return YMO_WOULDBLOCK;
    } else {
        YMO_NET_TRACE("%i: Head is NULL. No more buckets to send", fd);
        return YMO_OKAY;
    }
}



