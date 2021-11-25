##=============================================================================
##
## libyimmo: mod/ws/test/Makefile.am
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

AM_TESTSUITE_SUMMARY_HEADER = ' for libyimmo_ws $(PACKAGE_VERSION)'

AM_CFLAGS=\
 	-I@top_srcdir@/include \
 	-I@top_srcdir@/lib \
 	-I@top_srcdir@/mod \
	-I@top_srcdir@/mod/http/include \
 	-I@top_srcdir@/mod/ws/include \
	@BSAT_CFLAGS@

LDADD=\
	$(top_builddir)/lib/libyimmo.la \
	$(top_builddir)/mod/http/libyimmo_http.la \
	$(top_builddir)/mod/ws/libyimmo_ws.la

AM_DEFAULT_SOURCE_EXT=.c
check_PROGRAMS=\
	test_ws_basic

TESTS=\
	test_ws_basic

# EOF

