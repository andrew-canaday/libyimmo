##=============================================================================
##
## libyimmo: src/core/Makefile.am
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

yimmo_coredir=@YIMMO_INCLUDEDIR@/core
yimmo_core_HEADERS=\
	ymo_assert.h \
	ymo_bucket.h \
	ymo_conn.h \
	ymo_net.h \
	ymo_proto.h \
	ymo_server.h \
	ymo_tap.h \
	ymo_test_proto.h \
	ymo_tls.h \
	ymo_trie.h


SUBDIRS=\
	test

lib_LTLIBRARIES=\
	libyimmo.la

# TODO: pthreads should be conditional
libyimmo_la_CFLAGS=\
	-I@top_srcdir@/src \
	-I@top_builddir@/include \
	-I@top_srcdir@/include \
	@BSAT_CFLAGS@ \
	@PTHREAD_CFLAGS@

# TODO: pthreads should be conditional
libyimmo_la_LDFLAGS=\
	-version-info @YMO_LIB_VERSION@ \
	@BSAT_LIBS@ \
	@LIBS@ \
	@PTHREAD_LIBS@

libyimmo_la_SOURCES=\
	ymo_alloc.c \
	ymo_server.c \
	ymo_conn.c \
	ymo_util.c \
	ymo_env.c \
	ymo_net.c \
	ymo_bucket.c \
	ymo_trie.c \
	ymo_proto.c \
	ymo_blalloc.c \
	ymo_queue.c \
	yimmo.c

pkgconfig_DATA=\
	@PACKAGE_NAME@-@YIMMO_VERSION_MAJOR@.@YIMMO_VERSION_MINOR@.pc

# NOTE: This will overwrite the major-version directory, even if installing
# a lesser minor version (e.g. installing v1.2 after previously insstalling
# v1.3 will result in libyimmo-1.pc -> libyimmo-1.2.pc).
install-data-hook:
	cd $(DESTDIR)@pkgconfigdir@ && \
		$(LN_S) -f \
			@PACKAGE_NAME@-@YIMMO_VERSION_MAJOR@.@YIMMO_VERSION_MINOR@.pc \
			@PACKAGE_NAME@-@YIMMO_VERSION_MAJOR@.pc

