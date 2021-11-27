# ===========================================================================
#    https://github.com/andrew-canaday/libyimmo/m4/ymo_check_socket_api.m4
# ===========================================================================
#
# SYNOPSIS
#
#   YMO_CHECK_SOCKET_API()
#
# DESCRIPTION
#
#    Macro used to determine what the socket API characteristics are for the
#    current platform.
#
# LICENSE
#
#   Copyright (c) 2015 Andrew T. Canaday <http://github.com/andrew-canaday>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.
#
#==============================================================================

#serial 2

# -------------------------------------------------------------------
# YMO_CHECK_SOCKET_API()
# -------------------------------------------------------------------
AC_DEFUN([YMO_CHECK_SOCKET_API],
[
  YMO_BOX([Checking socket API: headers])
  AC_CHECK_HEADERS([ev.h fcntl.h sys/ioctl.h])
  AC_CHECK_HEADERS([sys/epoll.h sys/event.h poll.h])
  AC_CHECK_FUNCS([epoll_ctl kqueue poll select])

  AS_IF([test "x$ac_cv_header_fcntl_h" == "xyes"],[
      fcntl_h_include="#include <fcntl.h>"
  ])

  AS_IF([test "x$ac_cv_header_sys_ioctl_h" == "xyes"],[
      ioctl_h_include="#include <sys/ioctl.h>"
  ])

  # Optional socket flags:
  ## (This is probably way-overkill):
  YMO_BOX([Checking socket API: Error codes and flags])
  AC_CHECK_DECLS([
          EAGAIN,
          EWOULDBLOCK,
          MSG_NOSIGNAL,
          MSG_DONTWAIT,
          O_NONBLOCK,
          O_NDELAY,
          FIONBIO,
          FNONBLOCK,
          F_GETNOSIGPIPE,
          F_SETNOSIGPIPE,
          TCP_CORK,
          TCP_DEFER_ACCEPT,
          TCP_INFO,
          TCP_KEEPALIVE,
          TCP_KEEPCNT,
          TCP_KEEPIDLE,
          TCP_KEEPINTVL,
          TCP_LINGER2,
          TCP_MAXRT,
          TCP_MAXSEG,
          TCP_NODELAY,
          TCP_STDURG,
          TCP_SYNCNT,
          TCP_WINDOW_CLAMP,
          SO_LINGER,
          SO_KEEPALIVE,
          SO_RECVBUF,
          SO_SENDBUF,
          SO_REUSEPORT,
          SO_REUSEPORT_LB,
          SO_REUSEADDR,
          SO_NOSIGPIPE],
      [],[],[
          #include <errno.h>
          #include <sys/socket.h>
          #include <netinet/tcp.h>
          $fcntl_h_include
          $ioctl_h_include
          ])

  # Gather/scatter IO API:
  YMO_BOX([Checking socket API: Gather/Scatter IO])
  AC_CHECK_DECLS([sendmsg],[],
          [YMO_ERROR([libyimmo build requires "$as_decl_name" definition])],
          [#include <sys/socket.h>])

  AS_IF([test "x$ac_cv_have_decl_sendmsg" != "xyes"],[
      YMO_ERROR([libyimmo requires the sendmsg syscall])
  ])

  ## (This is probably way-overkill):
  AC_CHECK_MEMBERS([
      struct msghdr.msg_name,
      struct msghdr.msg_namelen,
      struct msghdr.msg_iov,
      struct msghdr.msg_iovlen,
      struct msghdr.msg_control,
      struct msghdr.msg_controllen,
      struct msghdr.msg_flags],[],
      [YMO_ERROR([unknown socket gather/scatter IO API])],
      [#include <sys/socket.h>])
])

# EOF

