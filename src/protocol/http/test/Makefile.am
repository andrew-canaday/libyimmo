##=============================================================================
##
## libyimmo: src/protocol/http/test/Makefile.am
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

AM_TESTSUITE_SUMMARY_HEADER = ' for libyimmo_http $(PACKAGE_VERSION)'

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  @top_srcdir@/build-aux/tap-driver.sh

AM_CFLAGS=\
	-D_DEFAULT_SOURCE \
	-I@top_srcdir@/src \
	-I@top_builddir@/include \
	-I@top_srcdir@/include \
	-I@top_srcdir@/src/protocol \
	-I@top_srcdir@/src/protocol/http \
	-I@top_srcdir@/src/protocol/http/include \
	@BSAT_CFLAGS@

AM_LDFLAGS=\
	@BSAT_LIBS@ \
	@LIBS@

LDADD=\
	$(top_builddir)/src/protocol/http/libyimmo_http.la \
	$(top_builddir)/src/core/libyimmo.la

AM_DEFAULT_SOURCE_EXT=.c
check_HEADERS=\
	ymo_http_test.h

check_PROGRAMS=\
	test_hdr_table \
	test_http_response \
	test_http_parser

TESTS=\
	test_hdr_table \
	test_http_response \
	test_http_parser

# EOF

