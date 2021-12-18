Yimmo HTTP Tests
================

This is sort of a hack...

There are two test "suites" (they are trivial):

 - [`test_http_1_0_basic.sh`](./test_http_1_0_basic.sh) (bash): Run some basic HTTP/1.0 checks against a runnning server.
 - [`test_http_basic.py`](./test_http_basic.py) (python3/requests/pytest): Run some basic HTTP/1.1 checks against a running server.

At the moment, tests are run against the [Yimmo WSGI Demo App](../../wsgi/demo).

