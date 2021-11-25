YIMMO
=====

Yimmo (yup, I made my own) is a streaming socket framework, that I made for the
fun of it (C is fun!).

The I/O architecture is a traditional reactor pattern which uses `libev`_ for
I/O readiness notifications.

.. contents:: Contents
   :local:
   :depth: 2

.. toctree::
   :maxdepth: 3
   :hidden:

   examples/index
   api
   wsgi/index
   maintainer
   genindex


.. _Core Build Settings:

Settings
--------

.. note::
   Yimmo has both *compile-time* and *runtime* settings.

   - Runtime settings have a ``YIMMO_`` prefix.
   - Compile-time settings have a ``YMO_`` prefix.

   Moving some of the compile-time parameters into the runtime is WIP.


Runtime
.......

.. list-table::
   :header-rows: 1
   :widths: auto
   :name: Runtime Settings

   * - Env Var
     - Description
     - Default
   * - ``YIMMO_LOG_LEVEL``
     - Log level (subject to ``YMO_LOG_LEVEL_MAX`` below).

       One of: ``FATAL``, ``ERROR``, ``WARNING``, ``NOTICE``, ``INFO``,
       ``DEBUG``, or ``TRACE``.
     - ``YMO_LOG_LEVEL_DEFAULT``
   * - ``YIMMO_SERVER_IDLE_TIMEOUT``
     - Idle disconnect timeout.
     - ``YMO_SERVER_IDLE_TIMEOUT``

Compile-Time
............

.. list-table::
   :header-rows: 1
   :widths: auto
   :name: Compile-time Settings

   * - ``configure`` var
     - Description
     - Default
   * - ``YMO_LOG_LEVEL_MAX``
     - Compile-time log-level max
     - ``NOTICE``
   * - ``YMO_LOG_LEVEL_DEFAULT``
     - Compile-time log-level default.
     - ``WARNING``
   * - ``YMO_SERVER_IDLE_TIMEOUT``
     - Default connection idle-disconnect timeout.
     - ``5``
   * - ``YMO_HTTP_RECV_BUF_SIZE``
     - maximum number of bytes allocated for headers, per-request.
     - ``1024``
   * - ``YMO_HTTP_SEND_BUF_SIZE``
     - maximum number of bytes allocated for headers, per-response.
     - ``1024``
   * - ``YMO_SERVER_RECV_BUF_SIZE``
     - the server read buffer.
     - ``8192``
   * - ``YMO_HTTP_REQ_WS_SIZE``
     - maximum WebSocket message *chunk* size.
     - ``1024``
   * - ``YMO_MQTT_RECV_BUF_SIZE``
     - maximum MQTT received payload size
     - ``4096``
   * - ``YMO_BUCKET_MAX_IOVEC``
     - maximum ``iovec`` array length for ``sendmsg()``
     - ``32``

