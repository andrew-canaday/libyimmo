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

##----------------------------
## Doc Build Variables:
##----------------------------
PLANTUML      ?= plantuml
SOURCEDIR     ?= @builddir@


##----------------------------
## Diagrams and Images:
##----------------------------
SUFFIXES: .svg .pu

.pu.svg: skinparams.iuml
if HAVE_PLANTUML
	$(PLANTUML) -nometadata -tsvg $(<) \
		&& mv -v @srcdir@/$(@) \
			@builddir@/$(@)
else
	@echo "ERROR: plantuml is required for diagram generation)"
	exit 1
endif

docs-local: yimmo-core.svg \
	yimmo-data.svg \
	yimmo-high-level.svg \
	yimmo-wsgi-spsw.svg \
	yimmo-wsgi-spmw.svg \
	yimmo-wsgi-mpsw.svg \
	yimmo-wsgi-mpmw.svg \
	yimmo-wsgi-events.svg \
	yimmo-wsgi-locks.svg

