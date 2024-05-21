.. _WSGI Server:

WSGI Server
===========

Yimmo-WSGI is a PEP3333 compliant WSGI server. It's written in C and built atop
the following libraries:

- `libev`_ for IO readiness notifications
- `libbsat`_ to manage timeouts
- `libyimmo`_ for network IO and HTTP

It `embeds`_ the Python interpretter (...I know, hang in).

The core I/O is handled using a traditional event-driven reactor pattern.
Application concurrency is provided by distributing request workloads across
a configurable number of processes and/or threads (both can also be ``1``).

The basic idea: use python to *handle* requests; use C for *I/O*.


.. _embeds: <https://docs.python.org/3/extending/embedding.html>

.. contents:: Contents
   :local:
   :depth: 2


.. toctree::
   :maxdepth: 1
   :hidden:

Building
--------

.. note::

   The Yimmo WSGI server is an optional build target, by default.

   To build the WSGI server from a default configuration, do:

   .. code-block:: sh

      # Do the build as usual:
      ./configure && make

      # Build the WSGI server:
      make -C ./wsgi yimmo-wsgi

   To enable the libyimmo WSGI server in *all builds*, pass ``--enable-wsgi``
   to ``configure``, e.g.:

   .. code-block:: sh

      # Make the WSGI server non-optional
      # at configure time:
      ./configure --enable-wsgi && make


Usage
-----


```
yimmo-wsgi LATEST
Usage: yimmo-wsgi [OPTIONS] WSGI_MODULE:WSGI_APP

Options:
  --config, -c     : Path to the yimmo-wsgi config yaml
  --log-level, -l  : Yimmo log level
  --port, -P       : Port
  --no-proc, -p    : Number of processes to run
  --no-threads, -t : Number of worker threads per proc
  --help, -h       : Display usage info (this)

Environment Variables:
  YIMMO_LOG_LEVEL           : libyimmo log level
  YIMMO_SERVER_IDLE_TIMEOUT : socket-level idle disconnect timeout
  YIMMO_WSGI_CONFIG         : yimmo wsgi config file path
  YIMMO_WSGI_NO_PROC        : number of yimmo WSGI processes to run
  YIMMO_WSGI_NO_THREADS     : number of worker threads per process
  YIMMO_WSGI_MODULE         : WSGI module (if not provided as arg)
  YIMMO_WSGI_APP            : WSGI app (if not provided as arg)
  YIMMO_TLS_CERT_PATH       : TLS certificate path
  YIMMO_TLS_KEY_PATH        : TLS private key path

Configuration precedence (greatest to least):
  - command line parameters
  - environment variables
  - config files

# Config Example:
---
log_level: INFO
wsgi:
  port: 8081
  tls:
    cert: /path/to/site.crt
    key: /path/to/cert.pem
  no_proc: 2
  no_threads: 2
```

Invocation
~~~~~~~~~~

The executable takes two arguments:

#. a module name
#. a python statement whose result is a WSGI app

```bash
yimmo-wsgi MODULE_NAME:INIT_STATMENT
```

Yimmo WSGI uses the standard interpretter python home and module search path.
If your module exists outside of the standard search path, you'll need to set
the ``PYTHONPATH`` environment variable.

Example
~~~~~~~

Suppose you had the following two flask apps, ``hello.py`` and
``hello_factory.py``, stored in ``/opt/my_apps``.

App Object
..........

If your module just contains an app object (e.g. a flask app or something), you
start it like so:

.. container:: two-col file-title

   .. code-block:: python
      :caption: hello.py

      from flask import Flask

      app = Flask(__name__)

      @app.route("/")
      def hello_world():
          return "<p>Hello, World!</p>"

   .. code-block:: sh
      :caption: Invocation

      # We'll run hello.py like this:
      PYTHONPATH=/opt/my_apps \
          yimmo-wsgi hello:app


App Factory
...........

If your module just has an app factory that takes zero or more parameters, you
can pass the invocation on the command line (**NOTE**: here, shell quoting is
important!), you start it like so:

.. container:: two-col file-title

   .. code-block:: python
      :caption: hello_factory.py

      from flask import Flask

      def create_app(config_filename):
          """
          (NOTE: Of course, this example is silly. Don't copy this pattern!)

          If you're interested in flask app factories, start here:
          https://flask.palletsprojects.com/en/2.0.x/patterns/appfactories/
          """

          app = Flask(__name__)
          print(f'\n\n***\nLoad some configuration from: {config_filename}\n***\n')

          @app.route("/")
          def hello_world():
              return '<p>Hello, world!</p>'

          return app

   .. code-block:: sh
      :caption: Invocation

      # We'll run hello.py like this:
      PYTHONPATH=/opt/my_apps \
          yimmo-wsgi 'hello_factory:create_app("my_config.cfg")'


Configuration
~~~~~~~~~~~~~

The following settings control the ``yimmo-wsgi`` runtime:

 - ``YIMMO_LOG_LEVEL``: libyimmo log level
 - ``YIMMO_SERVER_IDLE_TIMEOUT``: socket-level idle disconnect timeout
 - ``YIMMO_WSGI_CONFIG``: yimmo wsgi config file path
 - ``YIMMO_WSGI_NO_PROC``: number of yimmo WSGI processes to run
 - ``YIMMO_WSGI_NO_THREADS``: number of worker threads per process
 - ``YIMMO_WSGI_MODULE``: WSGI module (if not provided as arg)
 - ``YIMMO_WSGI_APP``: WSGI app (if not provided as arg)
 - ``YIMMO_TLS_CERT_PATH``: TLS certificate path
 - ``YIMMO_TLS_KEY_PATH``: TLS private key path
 - ``YIMMO_WSGI_PORT``: The port number to bind to.
 - ``YIMMO_WSGI_USE_KQUEUE``: Set to ``1`` on BSD-like systems to use ``kqueue`` (defaults to ``0``, wich falls back to using ``select()``).


TLS
...

If TLS is enabled, you can pass the server certificate and private key *paths*
using, ``YIMMO_TLS_CERT_PATH`` and ``YIMMO_TLS_KEY_PATH``.

.. _WSGI Logging:

Logging
~~~~~~~


.. warning::
   - At the moment, logging is implemented as line-buffered writes to ``stderr``.
   - ``yimmo-wsgi`` log levels do *not impact the python* ``logging`` *levels.*

Levels
......

.. list-table::
   :header-rows: 1
   :widths: auto

   * - Level Name
     - Description
   * - ``FATAL``
     - An event has occurred which is considered unrecoverable. The process is
       likely to abort, if it hasn't already.
   * - ``ERROR``
     - An event has occurred which limits or prevents normal operation for at least
       one connection.
   * - ``WARNING``
     - An error may occur if no corrective action is taken. You should probably
       have a look at it.
   * - ``NOTICE``
     - An anomalous or noteworthy event has taken place.
   * - ``INFO``
     - General, non-error, non-developer information.
   * - ``DEBUG``
     - General, non-error, developer information.
   * - ``TRACE``
     - This is a log level intended primarily for yimmo development. It provides
       fine-grained logging of some of the internal functions.

.. note::
   Log levels can be configured at runtime, but *only within* the range built-in
   at compile time — see ``YMO_LOG_LEVEL_MIN`` and ``YMO_LOG_LEVEL_MAX`` in the
   `core build parameters documentation <index.html>`_.


Concurrency
-----------

Every *worker process* has *at least two* threads:

 #. One **"server thread"** that does I/O using yimmo, ev, and bsat.
 #. One or more **"worker threads"** that handle requests (providing the PEP3333 interface to the application).

Queues are used to ferry HTTP information between the two.

There are some locks involved. It is tolerable. Don't worry.

.. include:: wsgi_concurrency.rst

Hey, what about gevent?
~~~~~~~~~~~~~~~~~~~~~~~

If your application is I/O bound in the request handlers themselves (e.g. lots
of reading/writing of files, or making additional HTTP requests to upstreams
as part of handling requests), you can happily monkey-patch `gevent`_
into your WSGI application, and it'll all work out just fine.

Summary
~~~~~~~

- **multi-processing**: *increases throughput* by reducing GIL contention.
- **multi-threading**: *reduce latency* (with a *very minor* decrease in throughput)
  incurred by *CPU-bound* request handlers, by allowing long-running and
  short-running requests to be handled concurrently.
- **gevent**: *reduce latency* incurred by *IO-bound* request handlers, by
  auto-patching non-blocking file and network I/O and lightweight cooperative
  multi-threading (`greenlets`_) into your app.

