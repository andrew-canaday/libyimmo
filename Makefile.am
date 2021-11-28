##=============================================================================
##
## libyimmo: Top-level automake file
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

.PHONY: fmt

ACLOCAL_AMFLAGS=-Im4

SUBDIRS=\
	include \
	lib \
	mod \
	example \
	wsgi \
	benchmarks \
	doc

pkgconfig_DATA=@PACKAGE_NAME@-@YIMMO_VERSION_MAJOR@.@YIMMO_VERSION_MINOR@.pc

# TODO: use 'install' to install things...

# NOTE: This will overwrite the major-version directory, even if installing
# a lesser minor version (e.g. installing v1.2 after previously insstalling
# v1.3 will result in libyimmo-1.pc -> libyimmo-1.2.pc).
install-data-hook:
	cd $(DESTDIR)@includedir@ && \
		$(LN_S) -f @YIMMO_INCLUDE@ yimmo-${YIMMO_VERSION_MAJOR}
	cd $(DESTDIR)@pkgconfigdir@ && \
		$(LN_S) -f \
			@PACKAGE_NAME@-@YIMMO_VERSION_MAJOR@.@YIMMO_VERSION_MINOR@.pc \
			@PACKAGE_NAME@-@YIMMO_VERSION_MAJOR@.pc

fmt:
	$(top_srcdir)/util/fmt-code.sh

## EOF

