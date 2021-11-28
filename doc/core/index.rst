.. _Core Overview:

Overview
========

.. toctree::
   :caption: API
   :maxdepth: 2
   :hidden:

   event_flow

Libyimmo has a simple, layered, architecture which compartmentalizes
functionality into the following three, general, domains:

- the transport (e.g. TCP) — handled by the :ref:`server <server>`
- the protocol (e.g. HTTP/WS) — handled by :ref:`protocols`
- the application — handled by user code

The aim is to keep the code for each layer as intelligible as possible by
limiting the scope of concerns — allowing the layer below to handle technical
intracacies and building on top of it to provide a higher level of abstraction
to the layer above.

So, for instance, the *protocol* doesn't have to worry about checking ``EAGAIN``
on ``send()``, fiddling with ``ioctl``, handling ``SIGPIPE``, etc. The *server*
doesn't have to concern itself with useir sessions, transactions, encoding, etc.
The *user* code doesn't have to worry about validating input, transfer encoding,
protocol versions, etc.

Interaction
...........

Each layer implements an interface, defined by the layer below it, and
exposes a set of utility functions which the layer above it can invoke to
call down.

**No layer should know anything about the technical details of the layer above
it, outside of the interface it defines**. So, for example, the TCP server
should *never need to know about or be concerned with protocol details* — e.g.
setting HTTP headers, checking protocol flags, etc.

.. admonition:: info

   For a diagrammatic view, see :ref:`Core Event Flow`.

Data
....

.. rst-class:: fig-right

   .. figure:: /diagrams/yimmo-data.svg
      :alt: ymo_proto_t and ymo_conn_t have data fields populated by
            specific protocol implementations. Usually, the protocol
            itself occupies the ymo_proto_t data field and the
            ymo_conn_t data field is leveraged by the protocl to
            store protocol-specific session information.
      :target: /_images/yimmo-data.svg


Most of the entity types in libyimmo have a ``data`` or ``user_data`` field
associated with them. These allow a given layer in the architecture to provide
a place for the layer above it to persist arbitrary data. At a high-level, data
can be associated with a server or with a particular connection. 

Generally speaking:

- The ``_create`` or ``_init`` function for one layer accepts a ``data`` parameter.
- The ``data`` parameter is passed to the layer above during callback invocations.
- The layer above stashes its layer-specific persistent data in that pointer.
  The layer-specific persistent data usually *also* has a ``data`` field,
  populated by that layer's ``_create`` or ``_init`` function.

etc, etc, etc. You get the gist. Each layer optionally provides the one above
it with a place to store data at init time and makes that data available
as a callback parameter.

The exact details vary by protocol, but here's a quick look at how this works
with the HTTP protocol, which should help the illustrate the gist of the thing:

Example: HTTP Data
^^^^^^^^^^^^^^^^^^

**Transport-Level**:

The :c:struct:`ymo_proto` provides a ``data`` for protocol implementations to
store data that will persist for the lifetime of the server, and
:c:struct:`ymo_conn` provides a ``user_data`` field that will persist for the
lifetime of the connection.

When the server invokes protocol callbacks, it passes the proto data field
and, where applicable, the connection ``user_data`` field.

**Protocol-Level**:

:c:func:`ymo_proto_http_create` populates the :c:struct:`ymo_proto` ``data``
field with its own, protocol-specific, data structure
(``ymo_http_proto_data``) in order to store things like the users's HTTP
header and body callbacks, upgrade handlers, etc.

It populates the :c:struct:`ymo_conn` ``data`` field with a
:c:struct:`ymo_http_session`, which is used to track things like HTTP
exchange state, client capabilities, etc.

**User-Level**:

The ``ymo_http_proto_data`` struct, has its *own* ``data`` field, which is
populated with the value passed by the user when they invoke
:c:func:`ymo_proto_http_create` or :c:func:`ymo_http_simple_init`. Similarly,
the :c:struct:`ymo_http_session` type has a ``user_data`` field which is
optionally populated by user code when its
:c:type:`ymo_http_session_init_cb_t` is invoked.

When the protocol invokes user callbacks, it passes the HTTP proto data field
and, where applicable, the HTTP session ``user_data`` field.


