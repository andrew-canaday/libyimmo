##=============================================================================
##
## libyimmo: wsgi/Makefile.am
##
##=============================================================================
#==============================================================================
#  libyimmo
#
#  Copyright (c) 2021 Andrew Canaday
# 
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#==============================================================================

# NOTE: to handle python function casting, add:
#           -Wno-error=cast-function-type
#           -Wno-error=bad-function-cas
# Side note:
#     -Wno-error=cast-function-type
#     -Wno-error=bad-function-cas
#
# HACK warning: including lib internals.
AM_CFLAGS=\
	-I@top_srcdir@/src \
 	-I@top_builddir@/include \
 	-I@top_srcdir@/include \
	-I@top_srcdir@/wsgi/include \
 	-I@top_srcdir@/src/protocol/http/include \
 	-I@top_srcdir@/src/protocol/ws/include \
	-Wno-error \
	@BSAT_CFLAGS@ \
	@PYTHON_CFLAGS@ \
	@PTHREAD_CFLAGS@

AM_LDFLAGS=\
	@PYTHON_LIBS@ \
	@PTHREAD_LIBS@

LDADD=\
	$(top_builddir)/src/core/libyimmo.la \
	$(top_builddir)/src/protocol/http/libyimmo_http.la \
	$(top_builddir)/src/protocol/ws/libyimmo_ws.la \
	$(top_builddir)/src/protocol/mqtt/libyimmo_mqtt.la

AM_DEFAULT_SOURCE_EXT=.c
if BUILD_WSGI
bin_PROGRAMS=yimmo-wsgi
else
EXTRA_PROGRAMS=yimmo-wsgi
endif

if BUILD_PY_WEBSOCKETS
yimmo_wsgi_SOURCES=\
	main.c \
	ymo_wsgi_cli.c \
	ymo_wsgi_mod.c \
	ymo_wsgi_server.c \
	ymo_wsgi_worker.c \
	ymo_wsgi_proc.c \
	ymo_wsgi_exchange.c \
	ymo_wsgi_session.c \
	ymo_wsgi_context.c \
	ymo_py_websockets.c \
	ymo_wsgi_util.c
else
yimmo_wsgi_SOURCES=\
	main.c \
	ymo_wsgi_cli.c \
	ymo_wsgi_mod.c \
	ymo_wsgi_server.c \
	ymo_wsgi_worker.c \
	ymo_wsgi_proc.c \
	ymo_wsgi_exchange.c \
	ymo_wsgi_session.c \
	ymo_wsgi_context.c \
	ymo_wsgi_util.c
endif

# EOF

