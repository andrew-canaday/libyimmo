.. _yimmo public API:

C API
=====

The libyimmo API is split into two primary domains:

The "public" API (this) defines types and functions necessary to *use* the core library or write software that depends on one or more modules.

The :ref:`"internal" API <internal API>` defines types and functions necessary to *extend* the core library, write module extensions, or create new protocols.

This is the API documentation for the libyimmo **public API**.

.. admonition:: Info

   To get a general sense for the principles of operation, see
   :ref:`Core Overview`.

.. toctree::
   :caption: Modules
   :maxdepth: 3
   :includehidden:

   core/api
   http/api
   ws/api
   mqtt/api


