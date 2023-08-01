.. _HTTP Overview:


The HTTP Protocol
=================

.. contents:: Contents
   :local:


Sessions & Exchanges
--------------------

.. rst-class:: fig-right

   .. figure:: /img/Yimmo-HTTP-Entities.png
      :alt: A Request/Response pair gets grouped into a box labeled,
            "exchange". All the exchanges for a connection are grouped into a
            box labeled "session."
      :target: _images/Yimmo-HTTP-Entities.png

      A request and it's associated response form an **exchange.**
      A session encapsulates all of the exchanges on a single TCP connection.


An "exchange" consists of one HTTP request-response pair. An *client* session
consists of all the exchanges made over a single TCP connection.

.. _HTTP Callbacks:

Callbacks
---------

The following callbacks are defined for the Yimmo HTTP protocol:

.. list-table::
   :header-rows: 1
   :widths: auto

   * - Callback Name
     - Type
     - Description
     - Optional
   * - Session Init
     - :c:type:`ymo_http_session_init_cb_t`
     - Invoked when a new HTTP session has begun (i.e. a new TCP connection has
       been made to the HTTP listen port).
     - Yes
   * - Header
     - :c:type:`ymo_http_cb_t`
     - Invoked when HTTP header parsing is complete, but before body data is
       received.
     - Yes
   * - Body Data
     - :c:type:`ymo_http_body_cb_t`
     - Invoked once for each chunk of body data received from the client.
     - Yes
   * - Complete
     - :c:type:`ymo_http_cb_t`
     - Invoked after the whole HTTP request has been received.
     - **No**
   * - Session Cleanup
     - :c:type:`ymo_http_session_cleanup_cb_t`
     - Invoked after HTTP session close.
     - Yes

Buffering
---------

The core of libyimmo is oriented around handling *streams* of data
efficiently — i.e. to facilitate use-cases where connections are potentially
long-lived and data is sent at potentially irregular intervals (e.g. HTTP
long-polling, WebSockets, etc).

In order to facilitate incremental processing of HTTP post bodies,
libyimmo_http provides both buffered and unbuffered HTTP body handling.

Whether HTTP bodies are buffered or unbuffered is determined based on the
``body_cb`` argument passed to :c:func:`ymo_proto_http_create`:

.. list-table::
   :header-rows: 1
   :widths: auto
   :name: ymo_proto_http_create buffering

   * - ``body_cb``
     - Buffered
     - ``request->body``
   * - ``NULL``
     - **Yes**
     - Request body data
   * - User-provided
     - **No**
     - ``NULL``

Buffered Requests
.................

If no body callback is provided, libyimmo_http will use its own, internal,
body callback, which buffers the HTTP message body (up to
:c:macro:`YMO_HTTP_MAX_BODY` bytes). In this case, the full body payload
is delivered as ``request->body`` when your :c:type:`ymo_http_cb_t` is
invoked.

Unbuffered Requests
...................

If you provide a body callback (:c:type:`ymo_http_body_cb_t`),
your body callback will be invoked for each "chunk" of content recieved from the
client.

For uploads using ``Transfer-Encoding: Chunked`` this corresponds
to the literal HTTP body chunks; When the upload is non-chunked, the
callback is invoked for each discrete block of data returned by the
``read()`` syscall invocation.

.. warning:: *Re: the body parameter*:

   If you provide a body callback (i.e. your HTTP protocol is
   configured in *unbuffered* mode), the ``request->body`` field will be
   ``NULL`` when your :c:type:`ymo_http_cb_t` is invoked!

   If you choose unbuffered mode *and* need to have the whole HTTP request body
   present at :c:type:`ymo_http_cb_t` invocation time, you'll need to buffer it
   yourself.


Generating Responses
--------------------

In order to facilitate asynchronous handling of HTTP requests, the HTTP protocol
implementation allows the :c:type:`ymo_http_cb_t` user callback to decide how
to proceed with the request via its return code:

.. list-table::
   :header-rows: 1
   :widths: auto
   :name: ymo_http_cb_t Return Value

   * - Value
     - Meaning
   * - ``YMO_OKAY``
     - the request was handled succesfully and user code has invoked
       :c:func:`ymo_http_response_finish` to prepare the response for
       transmission.
   * - ``EAGAIN`` / ``EWOULDBLOCK``
     - the response will be generated asynchronously; user-code will notify
       when some or all of it is ready for transmission.
   * - All other values
     - the return code is interpretted as an ``errno`` value. The connection
       will be closed at the TCP-level (i.e. no HTTP error response is sent).


Transfer-Encoding and Content-Length
....................................

HTTP response bodies are assembled by one or more calls to :c:func:`ymo_http_response_body_append`,
followed by one call to :c:func:`ymo_http_response_finish`.

After yimmo has parsed and received an entire HTTP request, it invokes the user
:c:type:`ymo_http_cb_t` to hand it off for response generation. When the user
HTTP callback returns, yimmo sets the ``Content-Length`` and ``Transfer-Encoding``
headers as follows:

If the user code has set ``Content-Length``:

  Yimmo will send the response headers and whatever data it has on hand, then
  move on to handle IO for other sockets. Subsequent invocations of
  :c:func:`ymo_http_body_append` will re-arm the IO watcher for the request. The
  response is complete once the user code invokes
  :c:func:`ymo_http_response_finish`.


If the user code has already called :c:func:`ymo_http_response_finish`:

  The length of the payload is known; the ``Content-Length`` header is set to
  the total length of the payload data provided by the user in calls to
  :c:func:`ymo_http_response_body_append`.  Chunked transfer encoding is *not*
  used.

If the user has appended data, but not "finished" the request and the client
supports chunked transfer encoding:

  Yimmo sets ``Transfer-Encoding: chunked`` and transmits whatever data it has
  on hand, then moves on to handle IO for other sockets.

  **NOTE**: *yimmo handles the chunked body formatting — prepending the chunk
  size octets and appending the CRLR to each chunk automatically.*

  Subsequent invocations of :c:func:`ymo_http_body_append` will re-arm the IO
  watcher for the request. The terminal chunk is automatically generated when
  :c:func:`ymo_http_response_finish` is invoked.

If the user has appended data, but not "finished" the request and the client
does not support chunked transfer encoding (i.e. HTTP 1.0 clients):


  **No data for this response is immediately transmitted.** Yimmo will move on
  to handling IO for other sockets until after the user code
  has called :c:func:`ymo_http_response_finish`. At this point, the content
  length is calculated from the total payload length, the ``Content-Length``
  header is set, and chunked transfer encoding is *not* used.


Synchronous Response Handling
.............................

The general pattern for handling HTTP requests with libyimmo_http is as follows:

1. The :c:type:`ymo_http_cb_t` callback is invoked.
2. The user code sets some response headers, the status code, and some response
   body.
3. The user invokes :c:func:`ymo_http_response_finish` and
   returns ``YMO_OKAY``, signalling to libyimmo that the response is ready for
   transmission.

Asynchronous Response Handling
..............................

Sometimes, we need to do some work asynchronously before the response
can be generated (e.g. fetch data from an upstream, read a portion of a file,
etc). In these cases, the pattern is:

1. The :c:type:`ymo_http_cb_t` callback is invoked.
2. The callback returns ``EAGAIN`` or ``EWOULDBLOCK`` [#f1]_, *effectively
   taking ownership of the events for this response until notifying yimmo
   otherwise.*

Before returning, in step **2**, the user code is free to queue up whatever data
it has ready at the moment (or none, if none is ready!) — *as long as it doesn't
"finish" the request*. i.e. Any of the following may be invoked during callback
invocation or deferred, at your convenience:

- :c:func:`ymo_http_response_set_status` / :c:func:`ymo_http_response_set_status_str`
- :c:func:`ymo_http_response_add_header` / :c:func:`ymo_http_response_insert_header`
- :c:func:`ymo_http_response_body_append`

Notifying Yimmo: Send-Readiness and Hand-back
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Yimmo automatically gets a readiness notification any time that data is appended
to the response body using :c:func:`ymo_http_response_body_append`.

Once the user code invokes :c:func:`ymo_http_response_finish`, Yimmo will resume
ownership of the response object, as if the whole thing was handled during the
HTTP callback.

IMPORTANT: libev and threads!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Generally speaking, libev can be utilized in the context of multiple threads
with relative ease — *as long as we abide by the cardinal rule:*

  ...you can use as many loops as you want in parallel, as long as there are
  no concurrent calls into any libev function with the same loop parameter.

  — `Marc Lehmann, libev documentation <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#THREADS_AND_COROUTINES>`_

*What does this mean for users of libyimmo?*

If you are handling asynchronous response preparation using some combination
of libev watchers: you're fine. Carry on!

(For the time being), **if you are delegating response handling to another
thread,** that thread *should not* invoke
:c:func:`ymo_http_response_body_append` or :c:func:`ymo_http_response_finish`
*directly!*

Instead, you should *hand off* the data to the main thread using an
`ev_async_watcher`_ [#f2]_ (this is what ``yimmo-wsgi`` does).

Caveats!
^^^^^^^^

.. note:: *Idle Disconnect Timeouts!*

   Even if you return ``EAGAIN`` or ``EWOULDBLOCK``, the idle disconnect
   timer for the *socket* is still in play!

   The only way to reset the idle disconnect timer is to invoke
   :c:func:`ymo_http_response_body_append` with some data that
   is ready to transmit or :c:func:`ymo_http_response_finish` to
   notify yimmo that the whole thing is ready to send.


Quirks
------

This section details some quirks and assumptions made by the yimmo HTTP code.

.. note::

   Many of the following are more order-of-implementation details than they are
   indicators of a cemented design philosophy. If they are or become
   problematic for your usage of the library, please feel free to surface it!


Character Constraints
.....................

The HTTP protocol parser (see :ref:`HTTP Parser`) does not completely constrain
some portions of the HTTP request (``method``, ``header-field``, ``URI``, etc)
to the character sets defined by the relevant RFC's [#f3]_.

The request is parsed to ensure that it satisfies the basic *form* of an HTTP
request — i.e. that the essential structure (version specification, whitespace
rules, etc) is correct and is very *diligent about bounds checking*.

However, the **characters** allowed by yimmo in some of these fields is a
*superset* of what is allowed by the standard [#f4]_ — e.g. check out this
unicode mischief:

.. code-block::
   :caption: 0x128077
   :class: file-title

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


.. admonition:: Summary

   If your application is using header field names or URI's verbatim and
   the presence of non-``ISO-8859-1`` characters are problematic, you will need
   (at least, for the time being) to validate them application-side.


Headers
.......

The current HTTP header hash table implementation relies on a hash function
which has no collisions over a set of 266 standard, common, and
not-super-uncommon-I-guess HTTP headers (``src/protocol/test/test_hdr_table.c``
for the list used in testing). This allows libyimmo_http to use the header
hash (prior to modulo) as a key and skip the usual ``strcmp``. [#f5]_

.. note:: *More TODO list items!*

   - **Short term**: the ability to provide custom hash and compare algorithms
     is WIP (partially implemented, but disabled by default).
   - **Medium term**: using a key string comparison as a backstop (i.e. the
     traditional approach!) is on the TODO list (with the option to disable,
     if performance dictates and use-case allows).

**In most cases, this is probably not a *huge* deal:**

- If you run the service behind a load balancer, there's likely already some
  sanitization/filtering happening before the requests hit yimmo.
- If you are using only standard (or relatively common) HTTP headers, the
  worst a malformed request can do is deprive itself of some useful
  information.

**In some specific cases, this strategy could be problematic:**

- If you have custom headers which collide with the standard(+) headers listed
  above, a collision will result in the two values getting concatenated as
  if they were part of the same header.
- If you are relying on a particular header being set by a proxy in some
  canonical way, don't constrain client headers to a known set, and the
  proxy prepends it's headers to the request, it is possible that a client
  could overwrite the canonical header set by the proxy.

.. [#f1] Platform dependent, though...less than it used to be? I feel like most
   places have both these days...

.. [#f2] Since this has potential to be a recurring use-case, API facilities
   that handle the inter-thread handoff and loop notification have been added
   to the TODO list.

.. [#f3] For more info, see:

   - `RFC 5234 (ABNF), appendix B.1 — Core Rules <https://datatracker.ietf.org/doc/html/rfc5234#appendix-B.1>`_
   - `RFC 7230 (HTTP 1.1 Syntax and Routing), section 1 — Syntax and Notation <https://datatracker.ietf.org/doc/html/rfc7230#section-1>`_
   - `RFC 7231 (HTTP 1.1 Semantics and Content), section 8.3.1 — Considerations for New Header Fields <https://datatracker.ietf.org/doc/html/rfc7231#section-8.3.1>`_
   - `RFC 5987 (Character Set and Language Encoding for HTTP Header Field Parameters <https://datatracker.ietf.org/doc/html/rfc5987>`_
   - And the ol' `RFC 822 (Format for APRA Internet Text Messages) <https://datatracker.ietf.org/doc/html/rfc822#section-3.1>`_

.. [#f4] Which should probably be accommodated using RFC 5987, but that's a
   TODO item for a different day...

.. [#f5] Though this yields a nice little performance boost, the honest genesis
   of this strategy was simply "that was good enough for round 2" (round 1 was
   using offset-indexed TRIE's — which is *very fast, indeed*, but inflexible
   in the presence of custom headers and the absence of facilities to compile
   the TRIE at startup. Also, the TRIE's really shine through when you have a
   large number of headers overall or a tight lexicographical packing of
   the headers that are recieved — neither of which is typical of an average
   HTTP request).

