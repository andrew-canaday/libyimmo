##=============================================================================
##
## libyimmo: mod/mqtt/test/Makefile.am
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
#  along with this program.  If not, see <mqtt://www.gnu.org/licenses/>.
#
#==============================================================================

AM_TESTSUITE_SUMMARY_HEADER = ' for libyimmo_mqtt $(PACKAGE_VERSION)'

AM_CFLAGS=\
 	-I@top_srcdir@/include \
 	-I@top_srcdir@/lib \
 	-I@top_srcdir@/mod \
	@BSAT_CFLAGS@

LDADD=\
	$(top_builddir)/lib/libyimmo.la \
	$(top_builddir)/mod/mqtt/libyimmo_mqtt.la

AM_DEFAULT_SOURCE_EXT=.c

# EOF

