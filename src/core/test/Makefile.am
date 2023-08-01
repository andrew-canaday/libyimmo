##=============================================================================
##
## libyimmo: src/core/test/Makefile.am
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

AM_TESTSUITE_SUMMARY_HEADER = ' for libyimmo $(PACKAGE_VERSION)'

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  @top_srcdir@/build-aux/tap-driver.sh

AM_CFLAGS=\
	-D_DEFAULT_SOURCE \
	-I@top_srcdir@/src \
	-I@top_builddir@/include \
	-I@top_srcdir@/include \
	@BSAT_CFLAGS@

LDADD=\
	$(top_builddir)/src/core/libyimmo.la

AM_DEFAULT_SOURCE_EXT=.c
check_PROGRAMS=\
	test_assert \
	test_basic \
	test_list \
	test_util \
	test_trie \
	test_yaml

TESTS=\
	test_assert \
	test_basic \
	test_list \
	test_util \
	test_trie \
	test_yaml

# EOF

