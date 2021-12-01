.. _maintainer_wsgi:

yimmo-wsgi
==========

Developer documentation for yimmo-wsgi.

.. toctree::
   :maxdepth: 2
   :hidden:

   yimmo_wsgi_h
   ymo_wsgi_proc_h
   ymo_wsgi_worker_h
   ymo_wsgi_server_h
   ymo_wsgi_exchange_h
   ymo_wsgi_session_h
   ymo_wsgi_context_h
   ymo_wsgi_mod_h
   ymo_wsgi_util_h
   ymo_wsgi_cli_h

.. contents:: Contents
   :local:
   :depth: 2


Source Layout
-------------

The ``yimmo-wsgi`` source is laid out like so:

.. list-table::
   :header-rows: 1
   :widths: auto

   * - Source
     - Contents
   * - ``main.c``
     - Runtime entrypoint.
   * - ``yimmo_wsgi.h``
     - Common macros and type declarations.
   * - ``ymo_wsgi_proc.h``/``ymo_wsgi_proc.c``
     - Process startup and control.
   * - ``ymo_wsgi_worker.h``/``ymo_wsgi_worker.c``
     - Thread worker implementation.
   * - ``ymo_wsgi_server.h``/``ymo_wsgi_server.c``
     - IO thread implmenentation.
   * - ``ymo_wsgi_exchange.h``/``ymo_wsgi_exchange.c``
     - HTTP Exchange type and functions.
   * - ``ymo_wsgi_session.h``/``ymo_wsgi_session.c``
     - Yimmo WSGI session type and functions.
   * - ``ymo_wsgi_context.h``/``ymo_wsgi_context.c``
     - C implementation of the python :py:class:`yimmo.Context` class.
   * - ``ymo_wsgi_mod.h``/``ymo_wsgi_mod.c``
     - Defines the :py:mod:`yimmo` module.
   * - ``ymo_wsgi_util.h``/``ymo_wsgi_util.c``
     - Miscelaneous utility functions.
   * - ``ymo_wsgi_cli.h``/``ymo_wsgi_cli.c``
     - Command line interface.

Thread Interaction
------------------

.. rst-class:: fig-right

   .. figure:: /img/Yimmo-HTTP-Entities.png
      :alt: A Request/Response pair gets grouped into a box labeled, "exchange". All the exchanges for a connection are grouped into a box labeled "session."
      :target: _images/Yimmo-HTTP-Entities.png

      A request and it's associated response form an **exchange.**
      A session encapsulates all of the exchanges on a single TCP connection.


Every *worker process* has *at least two* threads:

#. One **"server thread"** that does I/O using yimmo, ev, and bsat.
#. One or more **"worker threads"** that handle requests (providing the PEP3333 interface to the application).

There are some locks and some reference counting involved. It is tolerable.
Don't worry.

Queues (:c:type:`ymo_queue_t`) are used to ferry HTTP information between the
two. Each worker thread has two queues, one for input, one for output. Each
queue has an associated lock (``pthread_mutex_t``).

The WSGI module implements an HTTP request handler callback
(:c:type:`ymo_http_cb_t`) which combines the request (:c:type:`ymo_http_request_t`)
and response (:c:type:`ymo_http_response_t`) into a single unit, the exchange
(:c:type:`ymo_wsgi_exchange_t`).

After creating the exchange, the callback locks the input queue for the worker
associated with the connection, enqueueus the exchange, and signals the worker
thread using a pthread condition variable.

Upon waking (or after completing the task at hand — the worker may have been
handling another request in parallel while ``libyimmo_http`` was doing IO) the
worker thread acquires the input queue lock, dequeues the exchange, acquires the
GIL, packages the encapsulated information up as a python ``yimmo.Context``,
and invokes the WSGI application, passing ``environ`` and
:py:meth:`yimmo.Context.start_response`.


.. include:: wsgi_concurrency.rst


Events
------

.. image:: /diagrams/yimmo-wsgi-events.svg
   :alt: Yimmo WSGI Events
   :align: center
   :target: ../_images/yimmo-wsgi-events.svg


Concurrency and Synchronization
-------------------------------

