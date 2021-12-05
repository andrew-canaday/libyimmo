.. _HTTP Overview:

Using the HTTP API
==================

.. contents:: Contents
   :local:

--------
Overview
--------

.. include:: overview.rst


------
Quirks
------

This section details some quirks and assumptions made by the yimmo HTTP code.

.. note::

   Many of the following are more order-of-implementation details than they are
   indicators of a cemented design philosophy. If they are or become
   problematic for your usage of the library, please feel free to surface it!


Character Constraints
.....................

The HTTP protocol parser (see :ref:`HTTP Parser`) does not constrain all input
characters to the valid set from the HTTP RFCs. The request is parsed to ensure
that it satisfies the basic *form* of an HTTP request — i.e. that the essential
structure (HTTP version, carriage returns and newlines, etc) is correct and is
very *diligent about bounds checking*. However, the **characters** allowed by
yimmo in some of these fields is a *superset* of what is allowed by the
standard — e.g.:

.. code-block::
   :caption: 0x128077

   *   Trying 127.0.0.1...
   * TCP_NODELAY set
   * Connected to 127.0.0.1 (127.0.0.1) port 8081 (#0)
   > GET /index.html HTTP/1.1
   > Host: 127.0.0.1:8081
   > User-Agent: curl/7.64.1
   > Accept: */*
   > thumbs-👍-up: 123
   >
   < HTTP/1.1 200 OK
   < content-type: text/html
   < Content-Length: 2
   <
   * Connection #0 to host 127.0.0.1 left intact
   OK* Closing connection 0


If your application is using header field names or URI's verbatim and non-ascii
characters are problematic, you will need (at least, for the time being) to
validate them application-side.


Headers
.......

The current HTTP header hash table implementation relies on a hash function
which has no collisions over a set of 266 standard, common, and
not-super-uncommon-I-guess HTTP headers (``src/protocol/test/test_hdr_table.c``
for the list used in testing).

Worth noting:

- Short term: the ability to provide custom hash and compare algorithms is WIP.
- Long term: using a key string comparison as a backstop (i.e. the usual
  approach!) is planned (with the option to disable, if performance dictates
  and use-case allows).

In most cases, this is probably not a *huge* deal:

- If you run the service behind a load balancer, there's likely already some
  sanitization/filtering happening before the requests hit yimmo.
- If you are using only standard (or relatively common) HTTP headers, the
  worst a malformed request can do is deprive itself of some useful
  information.

In some cases, this *could be* problematic:

- If you have custom headers which collide with the standard(+) headers listed
  above, a collision will result in the two values getting concatenated as
  if they were part of the same header.
- If you are relying on a particular header being set by a proxy in some
  canonical way, don't constrain client headers to a known set, and the
  proxy prepends it's headers to the request, it is possible that a client
  could overwrite the canonical header set by the proxy.

