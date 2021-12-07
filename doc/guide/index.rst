.. _C API Usage:

Guide to the C API
==================

This section gives a high-level view of libyimmo usage, architecture,
and development files. It isn't a tutorial (*coming soon!*), just some
supplementary information that ought to make the API docs [#f1]_ more intelligible.

.. toctree::
   :maxdepth: 3
   :caption: Sections

   overview
   settings
   files
   http

The Basics
----------

At a high-level, this is the way it works:

1. Choose a protocol (HTTP, Websockets, etc)
2. Define each of the required callbacks required by the protocol (e.g. see :ref:`HTTP Overview`)
3. Create a protocol object (e.g. :c:func:`ymo_proto_http_create`), passing in your callbacks.
4. Create a server object (:c:func:`ymo_server_create`), passing in your protocol and a :c:struct:`ymo_server_config`.
5. Prep the server for startup by invoking :c:func:`ymo_server_init`.
6. Start you event loop with `ev_run <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#SYNOPSIS>`_.

----

.. [#f1] (WIP, also — like everything else here!)
