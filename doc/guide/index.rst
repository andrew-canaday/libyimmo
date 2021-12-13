.. _C API Usage:

Guide to the C API
==================

This section gives a high-level view of libyimmo usage, architecture,
and development files. It isn't a tutorial (*coming soon!*), just some
supplementary information that ought to make the API docs [#f1]_ more intelligible.

.. admonition:: The Basics
   :class: admonition-info

   At a high-level, this is the way it works:

   1. Implement each of the callbacks required by some protocol

      (e.g. see :ref:`HTTP Overview`)
   2. Create a protocol object, passing in your callbacks.

      (e.g. :c:func:`ymo_proto_http_create`)
   3. Create, initialize, and start your server

      (:c:func:`ymo_server_create`, :c:func:`ymo_server_init`, :c:func:`ymo_server_start`)
   4. Start your event loop with `ev_run <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#SYNOPSIS>`_.


.. toctree::
   :maxdepth: 3
   :caption: Sections

   overview
   settings
   files
   http

.. [#f1] (WIP, also — like everything else here!)
