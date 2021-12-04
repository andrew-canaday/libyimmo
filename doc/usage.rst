.. _C API Usage:

Using the C API
===============

.. contents:: Contents
   :local:
   :depth: 2

.. toctree::
   :maxdepth: 3
   :hidden:

   settings


Installed Files
---------------


Header Files and Libs
.....................

**NOTE**: Here:

- ``${M}`` means "*package* major version"
- ``${m}`` means "*package* minor version"
- ``*`` is a wildcard (don't try to include ``ymo_*.h``)
- ``${dext}`` is your platform lib extension (e.g. ``.so`` or ``.dylib``, etc)

.. code-block::

   ${prefix}
   ├── bin                     ───┐
   │   ├── yimmo-example-http     ├─────  Binaries Installed Here
   │   ├── yimmo-example-mqtt     │
   │   ├── yimmo-example-ws       │
   │   └── yimmo-wsgi          ───┘
   │
   ├── include
   │   │
   │   └── yimmo-${M}.${m}
   │       ├── yimmo.h  ───┬─────  Core Public API
   │       ├── ymo_*.h  ───┘
   │       │
   │       ├── core        ───┬─────  Core Internal API
   │       │   └── ymo_*.h ───┘       (#include <core/ymo_*.h>)
   │       │
   │       │
   │       ├── ymo_http.h  ─────────  HTTP Public API
   │       │
   │       ├── http              ───┬─────  HTTP Internal API
   │       │   └── ymo_http_*.h  ───┘       (#include <http/ymo_http_*.h>)
   │       │
   │       │
   │       ├── ymo_ws.h  ─────────  WebSocket Public API
   │       │
   │       ├── ws              ───┬─────  WebSocket Internal API
   │       │   └── ymo_ws_*.h  ───┘       (#include <ws/ymo_ws_*.h>)
   │       │
   │       │
   │       ├── ymo_mqtt.h  ─────────  MQTT Public API
   │       │
   │       └── mqtt              ───┬─────  MQTT Internal API
   │           └── ymo_mqtt_*.h  ───┘       (#include <mqtt/ymo_mqtt_*.h>)
   │
   └── lib
       │                                    Core libs:
       ├── libyimmo.${M}.${dext}  ───────── dynamic
       ├── libyimmo.a             ───────── static
       ├── libyimmo.la            ───────── libtool archive
       │
       ├── libyimmo_http.${M}.${dext}  ───────── (Ditto http)
       ├── libyimmo_http.a
       ├── libyimmo_http.la
       │
       ├── ... (ditto "ws" and "mqtt")
       │
       └── pkgconfig
           └── ...

Pkg-Config
..........

Pkg-Config_ files are provided and installed into ``${prefix}/lib/pkgconfig``
for each major/minor version pair (with a symlink which points to the latest
major installed), e.g.:


.. code-block::

   ${prefix}
   │
   ├── include
   │   │
   │   └── ...
   │
   └── lib
       │
       │
       ├── ...
       │
       └── pkgconfig                         ───┐
           │                                    │
           ├── libyimmo-${M}.${m}.pc            ├── pkg-config data
           ├── libyimmo-${M}.${m}_http.pc       │   (one per lib)
           ├── libyimmo-${M}.${m}_mqtt.pc       │
           └── libyimmo-${M}.${m}_ws.pc      ───┘


These can be used to determine the ``CFLAGS`` and ``LDFLAGS`` to use when
building against libyimmo, e.g.:

.. code-block:: sh

   # CFLAGS
   $ pkg-config libyimmo-0 libyimmo_http-0 --cflags
   -I/usr/local/include/yimmo-0.0

   # LDFLAGS
   $ pkg-config libyimmo-0 libyimmo_http-0 --libs
   -L/usr/local/lib -lyimmo -lyimmo_http


Autotools
.........

If your project uses autotools, we can do one better, using PKG_CHECK_MODULES_.
To build a project which uses yimmo core and HTTP, for example, you might do:

.. code-block::
   :caption: configure.ac

   PKG_CHECK_MODULES([YIMMO], [libyimmo])
   PKG_CHECK_MODULES([YIMMO_HTTP], [libyimmo_http])

.. code-block:: make
   :caption: Makefile.am

   AM_CFLAGS=\
   	@YIMMO_CFLAGS@ \
   	@YIMMO_HTTP_CFLAGS@

   AM_LDFLAGS=\
   	@YIMMO_LIBS@ \
   	@YIMMO_HTTP_LIBS@

   bin_PROGRAMS=\
   	my-program

   my_program_SOURCES=\
   	main.c



Symlinks
........

Header directories, library files, and package config data are installed with
major/minor suffixes. Symlinks are created from the major version to the
major/minor pair, e.g.:

.. code-block::

   ${prefix}
   │
   ├── include
   │   └── yimmo-${M} -> yimmo-${M}.${m}
   │
   └── lib
       ├── libyimmo.${dext} -> libyimmo.${M}.${dext}
       ├── ...
       │
       └── pkgconfig
           │
           ├── libyimmo-${M}.pc -> libyimmo-${M}.${m}.pc
           └── ...


