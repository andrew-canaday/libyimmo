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




/** Network I/O
 * =============
 *
 */

#ifndef YMO_NET_H
#define YMO_NET_H
#include "ymo_config.h"

#include <sys/socket.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_bucket.h"

/*---------------------------------------------------------------*
 * Socket Flags:
 *---------------------------------------------------------------*/

/** Read/write-time no-sigpipe flag: */
#if HAVE_DECL_MSG_NOSIGNAL
#define YMO_MSG_NOSIGNAL MSG_NOSIGNAL
#else
#define YMO_MSG_NOSIGNAL 0
#endif /* MSG_NOSIGNAL */

/** Read/write-time non-blocking flag: */
#if HAVE_DECL_MSG_DONTWAIT
#define YMO_MSG_DONTWAIT MSG_DONTWAIT
#else
#define YMO_MSG_DONTWAIT 0
#endif /* MSG_DONTWAIT */

/** Default send flags: no sigpipe, non-blocking (if available): */
#define YMO_SEND_FLAGS (YMO_MSG_NOSIGNAL | YMO_MSG_DONTWAIT)

/** Default recv flags: no sigpipe, non-blocking (if available): */
#define YMO_RECV_FLAGS (YMO_MSG_NOSIGNAL | YMO_MSG_DONTWAIT)

/** Convenience macro for checking blocking operation
 */
#if HAVE_DECL_EAGAIN && HAVE_DECL_EWOULDBLOCK
/* EAGAIN and EWOULDBLOCK are present: */
    #define YMO_IS_BLOCKED(status) \
    (status == EAGAIN || status == EWOULDBLOCK)
    #define YMO_WOULDBLOCK EWOULDBLOCK

#elif HAVE_DECL_EAGAIN
/* Only EAGAIN is present: */
    #define YMO_IS_BLOCKED(status) \
    (status == EAGAIN)
    #define EWOULDBLOCK EAGAIN
    #define YMO_WOULDBLOCK EAGAIN

#elif HAVE_DECL_EWOULDBLOCK
/* Only EWOULDBLOCK is present: */
    #define YMO_IS_BLOCKED(status) \
    (status == EWOULDBLOCK)
    #define EAGAIN EWOULDBLOCK
    #define YMO_WOULDBLOCK EWOULDBLOCK

#endif

#if HAVE_DECL_MSG_DONTWAIT
#define ymo_client_sock_nonblocking(x) (0)
#else
#define ymo_client_sock_nonblocking(x) ymo_sock_nonblocking(x)
#endif /* HAVE_DECL_MSG_DONTWAIT */

#if HAVE_DECL_MSG_NOSIGNAL
#define ymo_client_sock_nosigpipe(x) (0)
#else
#define ymo_client_sock_nosigpipe(x) ymo_sock_nosigpipe
#endif /* MSG_NOSIGNAL */

#define YMO_SSL_WANT_READ(x) (x == SSL_ERROR_WANT_READ)
#define YMO_SSL_WANT_WRITE(x) (x == SSL_ERROR_WANT_WRITE)
#define YMO_SSL_WANT_RW(x) (YMO_SSL_WANT_READ(x) || YMO_SSL_WANT_WRITE(x))

/**---------------------------------------------------------------
 * Socket I/O
 *---------------------------------------------------------------*/

/** Send a bucket chain over the socket given by ``fd``.
 */
ymo_status_t ymo_net_send_buckets(int fd, ymo_bucket_t** head);

/**---------------------------------------------------------------
 * Socket Trait Functions
 *---------------------------------------------------------------*/

/** Put socket in non-blocking mode at the fd-level:
 *
 * - Use ``MSG_DONTWAIT`` (client fd's only) when available
 * - ``O_NONBLOCK`` (more consistent behavior; requires 2 syscalls)
 * - ``FIOBIO`` (less consistent across platforms; 1 syscall)
 */
static inline int ymo_sock_nonblocking(int fd)
{
    int flags;
#if HAVE_FCNTL_H && HAVE_DECL_O_NONBLOCK
    if( -1 == (flags = fcntl(fd, F_GETFL, 0))) {
        flags = 0;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#elif HAVE_SYS_IOCTL_H && HAVE_DECL_FIONBIO
    flags = 1;
    return ioctl(fd, FIONBIO, &flags);
#else
    return EINVAL;
#endif /* top */
}


/** SIGPIPE strategies, in order:
 *
 * - Use ``MSG_NOSIGNAL`` (client fd's only) when available
 * - ``SO_NOSIGPIPE`` (mostly only available on BSD and BSD-like systems)
 * - ``F_GETNOSIGPIPE``/``F_SETNOSIGPIPE``
 *    (Mac OS X; uses ``SO_NOSIGPIPE`` under covers)
 *
 * Otherwise, the user has to ignore sigpipe at the thread/process level
 * prior to reads/writes.
 */
static inline int ymo_sock_nosigpipe(int fd)
{
#if HAVE_DECL_SO_NOSIGPIPE
    int flag = 1;
    return setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &flag, sizeof(flag));
#elif HAVE_FCNTL_H && HAVE_DECL_F_SETNOSIGPIPE
    return fcntl(fd, F_SETNOSIGPIPE, 1);
#elif HAVE_SYS_IOCTL_H && HAVE_DECL_FIONBIO
    int flags = 1;
    return ioctl(fd, FIONBIO, &flags);
#else
    ymo_log_warning("No platform support for blocking sigpipe at the FD level");
    return EINVAL;
#endif /* HAVE_DECL_SO_NOSIGPIPE */
}


/** Set reuse address/port options.
 */
static inline int ymo_sock_reuse(int fd, ymo_server_config_flags_t flags)
{
    int sock_flag = 1;
    int r_val = 0;
    if( flags & YMO_SERVER_REUSE_ADDR ) {
#if HAVE_DECL_SO_REUSEADDR
        errno = 0;
        if( (r_val = setsockopt(
                fd, SOL_SOCKET, SO_REUSEADDR, &sock_flag, sizeof(sock_flag))) ) {
            int e_val = errno;
            ymo_log(YMO_LOG_ERROR, "SO_REUSEADDR failed with %i", r_val);
            return e_val;
        }
#else
        ymo_log(YMO_LOG_WARNING, "config->flags & YMO_SERVER_REUSE_ADDR," \
                "but SO_REUSEADDR is not available on this platform");
#endif /* HAVE_DECL_SO_REUSEADDR */
    }

    if( flags & YMO_SERVER_REUSE_PORT ) {
#if HAVE_DECL_SO_REUSEPORT
        errno = 0;
        if( (r_val = setsockopt(
                fd, SOL_SOCKET, SO_REUSEPORT,
                &sock_flag, sizeof(sock_flag))) ) {
            int e_val = errno;
            ymo_log(YMO_LOG_ERROR, "SO_REUSEPORT failed with %i", r_val);
            return e_val;
        }
#else
        ymo_log(YMO_LOG_WARNING, "config->flags & YMO_SERVER_REUSE_PORT," \
                "but SO_REUSEPORT is not available on this platform");
#endif /* HAVE_DECL_SO_REUSEPORT */
    }
    return YMO_OKAY;
}

#endif /* YMO_NET_H */

