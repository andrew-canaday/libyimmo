##=============================================================================
##
## libyimmo: example/Makefile.am
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


YIMMO_EXAMPLE_DOCDIR ?= $(srcdir)/doc
POMD4C               ?= pomd4c


AM_CFLAGS=\
 	-I@top_builddir@/include \
 	-I@top_srcdir@/include \
 	-I@top_srcdir@/mod \
 	-I@top_srcdir@/mod/http/include \
 	-I@top_srcdir@/mod/ws/include \
 	-I@top_srcdir@/mod/mqtt/include \
 	-I@top_srcdir@/example/include \
	-I@srcdir@ \
	@BSAT_CFLAGS@

LDADD=\
	$(top_builddir)/lib/libyimmo.la \
	$(top_builddir)/mod/http/libyimmo_http.la \
	$(top_builddir)/mod/ws/libyimmo_ws.la \
	$(top_builddir)/mod/mqtt/libyimmo_mqtt.la

AM_DEFAULT_SOURCE_EXT=.c
if BUILD_EXAMPLES
bin_PROGRAMS=\
	yimmo-example-http \
	yimmo-example-ws \
	yimmo-example-mqtt
else
EXTRA_PROGRAMS=\
	yimmo-example-http \
	yimmo-example-ws \
	yimmo-example-mqtt
endif

yimmo_example_http_SOURCES=http_example.c
yimmo_example_ws_SOURCES=ws_example.c
yimmo_example_mqtt_SOURCES=mqtt_example.c

docs-local: doc/HTTP.md doc/WS.md doc/MQTT.md

doc/HTTP.md: http_example.c
	$(POMD4C) $(srcdir)/http_example.c \
		> $(YIMMO_EXAMPLE_DOCDIR)/HTTP.md

doc/WS.md: ws_example.c
	$(POMD4C) $(srcdir)/ws_example.c \
		> $(YIMMO_EXAMPLE_DOCDIR)/WS.md

doc/MQTT.md: mqtt_example.c
	$(POMD4C) $(srcdir)/mqtt_example.c \
		> $(YIMMO_EXAMPLE_DOCDIR)/MQTT.md

