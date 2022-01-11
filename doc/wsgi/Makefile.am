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
		@top_srcdir@/wsgi/yimmo_wsgi.h \
		@top_srcdir@/wsgi/ymo_py_websockets.h \
		@top_srcdir@/wsgi/ymo_wsgi_cli.h \
		@top_srcdir@/wsgi/ymo_wsgi_context.h \
		@top_srcdir@/wsgi/ymo_wsgi_exchange.h \
		@top_srcdir@/wsgi/ymo_wsgi_mod.h \
		@top_srcdir@/wsgi/ymo_wsgi_proc.h \
		@top_srcdir@/wsgi/ymo_wsgi_proc_main.h \
		@top_srcdir@/wsgi/ymo_wsgi_proc_worker.h \
		@top_srcdir@/wsgi/ymo_wsgi_server.h \
		@top_srcdir@/wsgi/ymo_wsgi_session.h \
		@top_srcdir@/wsgi/ymo_wsgi_util.h \
		@top_srcdir@/wsgi/ymo_wsgi_worker.h
	cp -v \
		@srcdir@/*.rst \
		@builddir@

