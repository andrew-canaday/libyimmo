General TODO
============

When possible, TODO items should be documented in the source.
Items in this document:

- TODO items regarding core architecture
- TODO items regarding interface changes
- **ALSO...**: TODO items that *should* really be in source...


+---------+------------+------------+-----------------+
| |/|     | |_|        | |x|        | |v|             |
+---------+------------+------------+-----------------+
| done    | not done   | won't do   | deprioritized   |
+---------+------------+------------+-----------------+
| ``|/|`` | ``|_|``    | ``|x|``    | ``|v|``         |
+---------+------------+------------+-----------------+

.. contents:: Contents
   :local:
   :depth: 2


Next Up
-------

- |_| Generic list/pool/queue types (ala kernel)
- |_| Most ``_creates`` need an ``_init`` (see `needs init`_)
- |_| Exchanges: why allocate request/response separately? It's the source of
      a lot of cache misses.
- |_| WSGI code cleanup
- |_| **Add** ``strcmp`` **to header hash table**
- |_| We work with lots of strings of known length — may some utilities here
      would save a lot of strncmp, etc.
- |_| Repurpose trie for HTTP routing?
- |_| Instrumentation?
- |_| Provide faster file handling (sendfile or caching)
- |_| ``EV_EMBED``, ``ev_realloc``, and faux-slab allocator example
- |_| Clean up logging
- |_| Make logging more configurable or use a 3rd party lib (zlog, etc)
- |/| Fix `ymo_base64_encode` memory leak!
- |/| Clean up configure.sh build time/number of checks!
- |/| UPDATE LICENSE INFO
- |/| Gather/Scatter IO
- |/| Object model
- |/| Write convenience functions for protocol startup (see below)
- |/| Docs on public vs private
- |/| Build options for development headers
- |/| Shift connection header table to hash?
- |x| Commit to glib/apr neither

TESTS!!!!
.........

- |_| Core unit tests
- |_| HTTP unit tests
- |_| WS unit tests
- |x| (MQTT unit tests)

.. note::
   - Protocol, e.g. HTTP request/response validation against a running server.
   - Facilities for testing custom protocols.

DOCS
....

- |_| Callbacks and return codes
- |_| Tidy ``c2sphinx`` + make public.
- |_| Use sphinx plantuml plugin instead of bash.
- |_| Consider using sphinx emoji instead of replacements file.
- |_| Stop abusing CSS/poor alabaster and make a proper theme.

Configuration
.............

- |_| Runtime configuration for the following compile-time options:
    - |/| ``YMO_SERVER_IDLE_TIMEOUT`` (``5``)
    - |_| ``YMO_SERVER_RECV_BUF_SIZE`` (``2``)
    - |_| ``YMO_HTTP_RECV_BUF_SIZE`` (``4``)
    - |_| ``YMO_HTTP_REQ_WS_SIZE`` (``4``)
    - |_| ``YMO_HTTP_SEND_BUF_SIZE`` (``4``)
    - |_| ``YMO_MQTT_RECV_BUF_SIZE`` (``6``)
    - |_| ``YMO_BUCKET_MAX_IOVEC`` (``2``)


Usability/Stability
...................

- |/| WS body buffering (optional)
- |_| HTTP expect handler, ala upgrade handler.
- |_| **clean up includes and include paths!**
- |_| Toss a ``strcmp`` or two into http header table!


.. _needs init:

Needs Init
..........

.. list-table::
   :header-rows: 1
   :widths: auto
   :name: Create functions that need an ``_init`` counterpart

   * - Status
     - Function
     - Notes
   * - |/|
     - ``ymo_queue_create``
     - Done
   * - |/|
     - ``void* ymo_wsgi_session_create``
     - Already done
   * - |_|
     - ``ymo_http_response_create``
     - ?
   * - |_|
     - ``ymo_blalloc_create``
     - ?
   * - |_|
     - ``ymo_http_hdr_table_create``
     - ?
   * - |x|
     - ``ymo_bucket_create``
     - Not necessary, atm.
   * - |x|
     - ``ymo_conn_create``
     - Not necessary, atm.
   * - |x|
     - ``ymo_http_exchange_create``
     - Not necessary, atm.
   * - |x|
     - ``ymo_http_session_create``
     - Not necessary, atm.
   * - |x|
     - ``ymo_mqtt_session_create``
     - Not necessary, atm.
   * - |x|
     - ``ymo_ws_session_create``
     - Not necessary, atm.
   * - |x|
     - ``ymo_wsgi_exchange_create``
     - Not necessary, atm.
   * - |v|
     - ``ymo_oitrie_create``
     - Low priority.
   * - |v|
     - ``ymo_proto_http_create``
     - Low priority.
   * - |v|
     - ``ymo_proto_mqtt_create``
     - Low priority.
   * - |v|
     - ``ymo_proto_ws_create``
     - Low priority.
   * - |v|
     - ``ymo_server_create``
     - Low priority.
   * - |v|
     - ``ymo_trie_node_create``
     - Low priority.
   * - |v|
     - ``ymo_trie_create``
     - Low priority.

General
-------

- |/| Prune logging statements (round 1)
- |/| Provide session UUID's
- |/| If ``MSG_DONTWAIT`` is defined, don't bother invoking ymo_sock_nonblocking()?
- |/| Cleanup bucket interface/PROTOCOL TRANSITIONS
- |_| Add restrict where appropriate
- |_| There's a lot of checking for standard symbols in ymo_check_socket_api;
   better to check to see if the socket API is SYS V, BSD, or POSIX and then
   assume accordingly?


Interface
---------

- |/| Provide bind/listen code
- |/| Eliminate two-struct http_request scheme
- |/| Clean up compressed header table generation
- |/| Decouple server and protocols:
   - |/| Move proto-specific read/write code into proto translation unit
   - |/| Add changeable proto pointer to session object
   - |/| Add primary protocol to server object + invoke init
   - |/| Add protocol destructor
   - |/| Make protocols run-time constructible by clients
- |/| Pluggable handler callbacks for upgrade requests

Core Architecture
-----------------

- |/| Leverage ``SO_REUSEPORT``
- |_| Add optional multi-threading support to core.
- |_| Add optional multi-process support to core? (Probably: *no*).
- |_| TLS support

Concurrency
...........
- |_| Thread pools — |/| for WSGI (*hacky, though*)
- |_| Create utility function to run a function in a thread with automatic
      ev_async cb
- |/| Decouple ev_loop / IO from python interpretter, CPU-wise
- |/| Re-use ymo_queue nodes for data exchange between the two threads to prevent repeated malloc/free
- **Pattern**:
    - |/| Configurable number of pre-fork workers
    - |/| Configurable number of threads per worker
    - |_| Configurable number of gevent greenlets per thread


IO Loops/Event System
.....................

- |/| Move per-session timeout management *into* session.

Non-blocking I/O
................

- |/| determine when to use fcntl vs ioctl
- |/| non-blocking accept
- |/| non-blocking recv
- |/| non-blocking send

Memory
......

- |/| Add compile-time allocator specification
- |/| Allow user to compile without g_slice (e.g. to use malloc/jemalloc, etc)
- |_| Reference counting for buckets (*maybe*)
- |_| User-specified allocators (just macros, atm; make ``weak`` symbols)

Performance considerations
..........................

- |/| Use prefix-code state machine for HTTP 1.0/1.1 header parsing
- |/| Use libbsat for timeout management to avoid fd-by-fd checks
- |_| **Clean up struct packing**

Stability/Error Handling
------------------------
- |/| Check for recv buffer bounds violations on headers

Server Startup
--------------
- |/| bind
- |/| accept
- |/| listen
- |/| startup/shutdown
- |/| configuration
- |/| logging (home grown for now; 3rd party later)

Libev Callbacks
...............

- |/| accept_cb
- |/| read_cb
- |/| idle disconnect timeout
   - |/| Standard HTTP idle disconnect timeout

Socket Writes
.............

- |_| Send interface:
   - |/| ``...send( ymo_bucket_t* )``
   - |/| ``...send( YMO_BUCKET_FROM_CPY(const char* data, size_t len) )``
   - |/| ``...send( YMO_BUCKET_FROM_REF(const char* data, size_t len) )``
   - |_| ``...send( ymo_bucket_from_file(FILE* fp) )``
   - |_| ``...send( ymo_bucket_from_socket(ymo_conn_t* conn) )``
- |/| Websocket write
- |/| Standard HTTP
   - |/| Header writes
   - |/| HTTP bodies

Python/WSGI
-----------
- |/| Make WSGI server
- |/| Connection freeing: hold off in loop thread (reference counting) or
      provide some cancellation mechanism to python thread

Protocols
---------

HTTP 1.0/1.1
............

- |/| facilities to handle all *standard* HTTP 1.0/1.1 headers
- |/| HTTP request pipelining
- |/| don't serialize response until it's ready to go *out* (i.e. there are no
   other responses ahead of it in the pipeline); this prevents *overwriting*
- |/| Close session after first non-keep-alive request served

Parsing
^^^^^^^

- |/| parse HTTP request line
- |/| header parse
   - |/| HTTP 1.0/1.1 differentiation for standard traits:
   - |/| Chunking
   - |/| Keep-alive

Body Parsing
^^^^^^^^^^^^

- |/| Buffered, fixed-size POST bodies:
   - |/| Parsing
   - |/| Callback
- |/| Un-buffered, fixed-size POST bodies:
   - |/| Parsing
   - |/| Callback
- |/| Buffered, chunked POST bodies:
   - |/| Parsing
   - |/| Callback
- |/| Un-buffered, chunked POST bodies:
   - |/| Parsing
   - |/| Callback

HTTP Compression
^^^^^^^^^^^^^^^^

- |_| (?) HTTP Compression (Does this mean Content- *and* Transfer-Encoding?)
   - |_| identity
   - |_| deflate
   - |_| gzip
   - |_| bzip2
- |_| Optional HTTP Compression Schemes:
   - |_| `sdch <http://lists.w3.org/Archives/Public/ietf-http-wg/2008JulSep/att-0441/Shared_Dictionary_Compression_over_HTTP.pdf>`_
   - |_| `xz <http://en.wikipedia.org/wiki/Xz>`_
   - |_| `lzma <http://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Markov_chain_algorithm>`_

Request/Response Interface
^^^^^^^^^^^^^^^^^^^^^^^^^^

- |/| Keep-alive
- |/| Content-Length
- |_| Content-Encoding
- |/| Transfer-Encoding
- |_| Standard Content Negotiation

Error Codes
^^^^^^^^^^^

- |/| Look for "Expect" header and send HTTP 100 appropriately
- The following all just clip the TCP connection (fix it!):
    - |_| Error out with 400 for malformed requests!
    - |_| Error out with 413 or 417 for excessive body size!
    - |_| Error out with 431 for excessive header content!
- |x| Multi-line header values. (Obsoleted).

Websockets
..........

- |/| Upgrade
- |/| Websocket read
- |/| Write
- |_| Extensions
    - |_| ``permessage-deflate``

MQTT
....

- |/| Basic parsing
- |_| Topic routing through updated libatra/mqrs
- |_| Clean up type names (follow your own rules!)
- |_| Move appropriate flags to ymo_mqtt.h

Source/Build/Link
.................

- |/| move built-in http handler to separate translation unit
- |/| http parser should not touch server internals
- |/| session should not touch server internals
- |/| create allocator header for customization
- |/| leverage source macros to enforce encapsulation
- |/| Add "--enable-maintainer-debug" flag for maintainer builds
- |/| cleanup superfluous functions
- |/| define undefined compiler extensions
- |/| stdint.h inttypes.h + correct format strings (mostly done)
- |_| *cleanup superfluous includes*
- |_| check for the presence of features.h/cdefs.h

C Dialect
^^^^^^^^^

(Pick one...)

- |/| ISO C11 (for now; may restrict to C99 or expand to GNUC)
- |x| ISO C89
- |x| ISO C99
- |x| GNU C89
- |x| GNU C99
- |x| GNU C11

Stdlib
^^^^^^

(Pick one...)

- |/| Portable Operating System Interface (POSIX)
- |/| ISO C11
- |x| ISO C89
- |x| ISO C99
- |x| Single Unix Specifiation (SUS)
- |x| X/Open Portability Guide (XPG)
- |x| System V Interface Definition (SVID)
- |x| Berkley Unix (BSD)
- |x| GNUC
- |x| Add C++ compatibility


Dependencies
------------

- |/| configuration for libev
- |/| configuration for libbsat
- |x| configuration for glib (removed)
- |/| configuration for uuid
- |_| configuration for OpenSSL (or other TLS lib).

