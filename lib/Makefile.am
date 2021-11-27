##=============================================================================
##
## libyimmo: lib/Makefile.am
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
SUBDIRS=\
	test

lib_LTLIBRARIES=\
	libyimmo.la

libyimmo_la_CFLAGS=\
 	-I@top_srcdir@/include \
 	-I@top_srcdir@/lib \
	@BSAT_CFLAGS@

libyimmo_la_LDFLAGS=\
	-version-info @YMO_LIB_VERSION@ \
	@BSAT_LIBS@ \
	@LIBS@

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

# EOF

