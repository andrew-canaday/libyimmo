Sessions & Exchanges
....................

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

Callbacks
.........

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
