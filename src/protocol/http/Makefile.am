##=============================================================================
##
## libyimmo: src/protocol/http/Makefile.am
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
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#==============================================================================

yimmo_mod_httpdir=@YIMMO_INCLUDEDIR@/http

yimmo_mod_http_HEADERS=\
	ymo_http_exchange.h \
	ymo_http_hdr_table.h \
	ymo_http_parse.h \
	ymo_http_response.h \
	ymo_http_session.h \
	ymo_proto_http.h


SUBDIRS=\
	include \
	test

lib_LTLIBRARIES=\
	libyimmo_http.la

libyimmo_http_la_CFLAGS=\
	-I@top_srcdir@/src \
 	-I@top_builddir@/include \
 	-I@top_srcdir@/include \
	-I@top_srcdir@/src/protocol \
	-I@top_srcdir@/src/protocol/http/include \
	@BSAT_CFLAGS@

# TODO: Do we pass "-module" here (potentially yielding a "bundle" on Mac OS X)
# or leave it as a shared library? Contrary to the docs, it looks like shared
# libs can be dlopened on Mac OS X. This also allows clients to go ahead and
# link against the shared lib (impossible with a module). Any downside to this?
libyimmo_http_la_LDFLAGS=\
	-version-info @YMO_LIB_VERSION@ \
	@BSAT_LIBS@

libyimmo_http_la_SOURCES=\
	ymo_proto_http.c \
	ymo_http_session.c \
	ymo_http_parse.c \
	ymo_http_hdr_table.c \
	ymo_http_exchange.c \
	ymo_http_response.c \
	ymo_http_util.c

pkgconfig_DATA=\
	@PACKAGE_NAME@_http-@YIMMO_VERSION_MAJOR@.@YIMMO_VERSION_MINOR@.pc

install-data-hook:
	cd $(DESTDIR)@pkgconfigdir@ && \
		$(LN_S) -f \
			@PACKAGE_NAME@_http-@YIMMO_VERSION_MAJOR@.@YIMMO_VERSION_MINOR@.pc \
			@PACKAGE_NAME@_http-@YIMMO_VERSION_MAJOR@.pc

# EOF

