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

.PHONY: \
	core \
	http \
	ws \
	mqtt \
	wsgi \
	images \
	examples \
	sphinx-html \
	check-docs \
	checklinks

SUBDIRS=\
	core \
	http \
	ws \
	mqtt \
	wsgi \
	examples \
	diagrams


##----------------------------
## Doc Build Variables:
##----------------------------
PYTHON3       ?= python3
SPHINXOPTS    ?=
SPHINXBUILD   = $(PYTHON3) -msphinx
SPHINXPROJ    = libyimmo
SOURCEDIR     ?= @builddir@
HTMLDOCSDIR   ?= @builddir@/_build/html
MANDOCSDIR    ?= @builddir@/_build/man
LINKCHECKDIR  ?= @builddir@/_build/link_check


##----------------------------
## Checks:
##----------------------------
##----------------------------
## Images:
##----------------------------
images:
	mkdir -p \
		@builddir@/img
	cp -v \
		@srcdir@/img/*.png \
		@builddir@/img/

##----------------------------
## Sphinx (HTML) docs:
##----------------------------

# Some horrible hacks here:
sphinx-html: images
if !HAVE_C2SPHINX
	@echo "c2sphinx is required for API documentation"
	exit 1
endif
	mkdir -p $(HTMLDOCSDIR)

	cp -v \
		@srcdir@/*.rst \
		@builddir@

	cp -rv \
		@srcdir@/notes/ \
		@builddir@/notes

	$(SPHINXBUILD) \
		-b html \
		-c @builddir@ \
		"$(SOURCEDIR)" \
		"$(HTMLDOCSDIR)" \
		$(SPHINXOPTS)

sphinx-man:
	$(SPHINXBUILD) \
		-b man \
		-c @builddir@ \
		"$(SOURCEDIR)" \
		"$(MANDOCSDIR)" \
		$(SPHINXOPTS)


checklinks: docs-local
	$(SPHINXBUILD) \
		-b linkcheck \
		"$(SOURCEDIR)" \
		$(LINKCHECKDIR)
		$(SPHINXOPTS)
	@echo
	@echo "Check finished. Report is in $(LINKCHECKDIR)."

check-docs: checklinks

docs-local: sphinx-html

manual-local: sphinx-man


