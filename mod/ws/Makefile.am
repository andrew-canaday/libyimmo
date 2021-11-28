##=============================================================================
##
## libyimmo: mod/ws/Makefile.am
##
##=============================================================================
#==============================================================================
#  libyimmo
#
#  Copyright (c) 2014 Andrew Canaday
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
#  along with this program.  If not, see <ws://www.gnu.org/licenses/>.
#
#==============================================================================

SUBDIRS=\
	include \
	test

lib_LTLIBRARIES=\
	libyimmo_ws.la

libyimmo_ws_la_CFLAGS=\
 	-I@top_builddir@/include \
 	-I@top_srcdir@/include \
 	-I@top_srcdir@/lib \
	-I@top_srcdir@/mod \
	-I@top_srcdir@/mod/ws/include \
	-I@top_srcdir@/mod/http/include \
	@OPENSSL_INCLUDES@ \
	@BSAT_CFLAGS@

# TODO: Do we pass "-module" here (potentially yielding a "bundle" on Mac OS X)
# or leave it as a shared library? Contrary to the docs, it looks like shared
# libs can be dlopened on Mac OS X. This also allows clients to go ahead and
# link against the shared lib (impossible with a module). Any downside to this?
libyimmo_ws_la_LDFLAGS=\
	-version-info @YMO_LIB_VERSION@ \
	@OPENSSL_LDFLAGS@ \
	@OPENSSL_LIBS@ \
	@BSAT_LIBS@

libyimmo_ws_la_SOURCES=\
	ymo_proto_ws.c \
	ymo_ws_parse.c \
	ymo_ws_session.c

# EOF

