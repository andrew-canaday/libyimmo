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

C2SPHINX     ?= c2sphinx

docs-local:
	cp -v \
		@srcdir@/*.rst \
		@builddir@
	$(C2SPHINX) \
		-e "EXT=md" \
		-e "DOCDIR=@builddir@" \
		-p @top_srcdir@/util/c2sphinx-post.sh \
		@top_srcdir@/example/http_example.c \
		@top_srcdir@/example/ws_example.c \
		@top_srcdir@/example/mqtt_example.c
