##=============================================================================
##
## libyimmo: benchmarks/Makefile.am
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

# TODO: get rid of this or tidy it...
AM_CFLAGS=\
	-I@top_srcdir@/src \
 	-I@top_builddir@/include \
 	-I@top_srcdir@/include \
 	-I@top_srcdir@/src/protocol/http \
 	-I@top_srcdir@/src/protocol/http/extra \
 	-I@top_srcdir@/src/protocol/http/include \
	-I@srcdir@

LDADD=\
    	@top_builddir@/src/core/libyimmo.la \
    	@top_builddir@/src/protocol/http/libyimmo_http.la

AM_DEFAULT_SOURCE_EXT=.c
if BUILD_BENCHMARKS
bin_PROGRAMS=\
	benchmark_trie
else
EXTRA_PROGRAMS=\
	benchmark_trie
endif

# EOF

