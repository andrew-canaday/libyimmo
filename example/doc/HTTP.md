# Example: HTTP Server

1. [Define an HTTP Callback](#1-define-an-http-callback)
2. [Create and Start the Server](#2-create-and-start-the-server)

<br />

Default port to listen on for http connections. 


```C
#define HTTP_PORT 8081
```

## 1. Define an HTTP callback
User-space HTTP callback invoked by ymo_http when a new request is received
from an HTTP session.

- `request`:  the yimmo object containing information on the request
- `response`:  the response object to be filled out

Return `YMO_OKAY` on success; a value from errno.h on error.

> **NOTE:** Yimmo sends all _payload_ data (i.e. message _bodies_) using
> Apache-style "bucket brigades" (`ymo_bucket_t`).

Set the content type to "text/html" and set the body using EXAMPLE_TEXT.


```C
static ymo_status_t test_http_callback(
        ymo_http_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        void* user_data)
{
    ymo_log_info("HTTP: >%s %s<", request->method, request->uri);

    if( request->content_length ) {
        ymo_log_info("Got body data: %s", request->body);
    }

    ymo_http_response_set_header(response, "content-type", "text/html");
    ymo_http_response_set_status_str(response, "200 OK");
    ymo_bucket_t* content = YMO_BUCKET_FROM_REF("OK", 2);

    ymo_http_response_body_append(response, content);
    ymo_http_response_finish(response);
    return YMO_OKAY;
}
```

## 2. Create and Start the Server!


```C
int main(int argc, char** argv)
{
    ymo_server_t* http_srv = NULL;

    /* Say hello */
    issue_startup_msg();

    ymo_http_upgrade_handler_t* handlers[2] = {
        ymo_http2_no_upgrade_handler(),
        NULL,
    };

    /* libev loop init: */
    struct ev_loop* loop = ev_default_loop(0);

    /* Create the HTTP server: */
    http_srv = ymo_http_simple_init(
            loop, HTTP_PORT, &test_http_callback, handlers, NULL);

    if( !http_srv ) {
        ymo_log_error("Unable to create server: %s", strerror(errno));
        return -1;
    }

    /* Install a sigint handler so we can terminate gracefully: */
    ev_signal sigint_watcher;
    ev_signal_init(&sigint_watcher, sigint_handler, SIGINT);
    ev_signal_start(loop, &sigint_watcher);

    /* Start it up! */
    ymo_server_start(http_srv, loop);
    ev_run(loop,0);
    ymo_log(YMO_LOG_INFO, "Shutting down!");
    ymo_server_free(http_srv);
    return 0;
}
```

