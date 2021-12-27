YIMMO
=====

Yimmo (yup, I made my own) is a streaming socket framework.

It favors ease of use and performance over feature richness. It provides
some basics and attempts to provide a flexible, intuitive interface upon which
more complicated functionality can be built.

It has a single-process, single-threaded operating model
(multi-proc/multi-thread support is WIP).  The I/O architecture is a traditional
reactor pattern which uses `libev`_ for I/O readiness notifications.

.. admonition:: Want to get the gist?
   :class: admonition-info

   - To get a feel for the **overall architecture**, see :ref:`Core Overview`.
   - For detailed information about **using the C library**, see :ref:`C API Usage`.
   - **Examples**: have a peek at the simple `HTTP Example Server`_ or `WebSocket
     Echo Server`_ to get a sense for the usage.
   - Interested in running the **WSGI server?** See :ref:`WSGI Server`.

.. contents:: Contents
   :local:
   :depth: 2

.. toctree::
   :maxdepth: 3
   :includehidden:
   :caption: Contents

   examples/index
   guide/index
   api
   wsgi/index
   maintainer/index
   genindex


