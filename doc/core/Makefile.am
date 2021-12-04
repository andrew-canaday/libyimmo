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
		-e "DOCDIR=@builddir@" \
		-p @top_srcdir@/util/c2sphinx-post.sh \
		@top_builddir@/include/yimmo.h \
		@top_srcdir@/include/ymo_env.h \
		@top_srcdir@/include/ymo_alloc.h \
		@top_srcdir@/include/ymo_blalloc.h \
		@top_srcdir@/include/ymo_log.h \
		@top_srcdir@/include/ymo_queue.h \
		@top_srcdir@/include/ymo_util.h
	$(C2SPHINX) \
		-s \
		-e "EXT=rst" \
		-e "DOCDIR=@builddir@" \
		-p @top_srcdir@/util/c2sphinx-post.sh \
		@top_srcdir@/src/core/ymo_trie.h \
		@top_srcdir@/src/core/ymo_conn.h \
		@top_srcdir@/src/core/ymo_bucket.h \
		@top_srcdir@/src/core/ymo_proto.h \
		@top_srcdir@/src/core/ymo_server.h \
		@top_srcdir@/src/core/ymo_assert.h \
		@top_srcdir@/src/core/ymo_tap.h \
		@top_srcdir@/src/core/ymo_net.h
	cp -v \
		@srcdir@/*.rst \
		@builddir@

