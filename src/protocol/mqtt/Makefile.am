##=============================================================================
##
## libyimmo: src/protocol/mqtt/Makefile.am
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

yimmo_proto_mqttdir=@YIMMO_INCLUDEDIR@/mqtt
yimmo_proto_mqtt_HEADERS=\
	ymo_mqtt_parse.h \
	ymo_mqtt_session.h \
	ymo_proto_mqtt.h


LDADD=\
	$(top_builddir)/src/core/libyimmo.la

SUBDIRS=\
	include \
	test

lib_LTLIBRARIES=\
	libyimmo_mqtt.la

libyimmo_mqtt_la_CFLAGS=\
	-I@top_srcdir@/src \
 	-I@top_builddir@/include \
 	-I@top_srcdir@/include \
	-I@top_srcdir@/src/protocol \
	-I@top_srcdir@/src/protocol/mqtt/include \
	-DYMO_SOURCE="\"$(<F)\"" \
	@BSAT_CFLAGS@

# TODO: Do we pass "-module" here (potentially yielding a "bundle" on Mac OS X)
# or leave it as a shared library? Contrary to the docs, it looks like shared
# libs can be dlopened on Mac OS X. This also allows clients to go ahead and
# link against the shared lib (impossible with a module). Any downside to this?
libyimmo_mqtt_la_LDFLAGS=\
	-version-info @YMO_LIB_VERSION@ \
	@BSAT_LIBS@

libyimmo_mqtt_la_SOURCES=\
	ymo_proto_mqtt.c \
	ymo_mqtt_parse.c \
	ymo_mqtt_session.c

pkgconfig_DATA=\
	@PACKAGE_NAME@_mqtt-@YIMMO_VERSION_MAJOR@.@YIMMO_VERSION_MINOR@.pc

install-data-hook:
	cd $(DESTDIR)@pkgconfigdir@ && \
		$(LN_S) -f \
			@PACKAGE_NAME@_mqtt-@YIMMO_VERSION_MAJOR@.@YIMMO_VERSION_MINOR@.pc \
			@PACKAGE_NAME@_mqtt-@YIMMO_VERSION_MAJOR@.pc


