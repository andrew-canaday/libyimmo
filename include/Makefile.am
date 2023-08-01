##=============================================================================
##
## libyimmo: include/Makefile.am
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

yimmodir=@YIMMO_INCLUDEDIR@
yimmo_HEADERS=\
	yimmo.h \
	ymo_alloc.h \
	ymo_attrs.h \
	ymo_blalloc.h \
	ymo_env.h \
	ymo_list.h \
	ymo_log.h \
	ymo_queue.h \
	ymo_util.h \
	ymo_yaml.h


# NOTE: This will overwrite the major-version directory, even if installing
# a lesser minor version (e.g. installing v1.2 after previously insstalling
# v1.3 will result in yimmo-1 -> yimmo-1.2).
install-data-hook:
	cd $(DESTDIR)@includedir@ && \
		$(LN_S) -f @YIMMO_INCLUDE@ yimmo-${YIMMO_VERSION_MAJOR}

# EOF

