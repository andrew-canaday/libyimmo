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
	$(C2SPHINX) \
		-s \
		-e "EXT=rst" \
		-p @top_srcdir@/util/c2sphinx-post.sh \
		@top_srcdir@/src/protocol/http/include/ymo_http.h \
		> @builddir@/api.rst
	$(C2SPHINX) \
		-s \
		-e "EXT=rst" \
		-e "DOCDIR=@builddir@" \
		-p @top_srcdir@/util/c2sphinx-post.sh \
		@top_srcdir@/src/protocol/http/ymo_http_parse.h \
		@top_srcdir@/src/protocol/http/ymo_proto_http.h \
		@top_srcdir@/src/protocol/http/ymo_http_hdr_table.h \
		@top_srcdir@/src/protocol/http/ymo_http_session.h \
		@top_srcdir@/src/protocol/http/ymo_http_exchange.h \
		@top_srcdir@/src/protocol/http/ymo_http_response.h
	cp -v \
		@srcdir@/*.rst \
		@builddir@

