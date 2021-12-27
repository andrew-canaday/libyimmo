WebSockets CI
=============

WebSockets are tested using the [Autobahn Test Suite](https://github.com/crossbario/autobahn-testsuite) and run inside a
[docker compose app](./docker-compose.yml).

#### Files

 - `docker-compose.yml`: Compose app consisting of Yimmo WS echo server and Autobahn test suite.
 - `autobahn-entrypoint.sh`: Entrypoint for the autobahn image in the compose setup.
 - `config`: Autobahn configuration json
 - `autobahn-json-summary.sh`: Given a path to an `index.json` file, print the results and generate a summary.
 - `autobahn-local.sh`: Used to run autobahn test suite against a WS server running on localhost.

## Test Suites

 - :white_check_mark: `1`: Framing
 - :white_check_mark: `2`: Pings/Pongs
 - :white_check_mark: `3.*`: Reserved Bits
 - :white_check_mark: `4.*`: Opcodes
 - :white_check_mark: `5.*`: Fragmentation
 - :white_large_square: `6.*`: UTF-8 Handling (_out of scope_ — see notes below)
 - :white_check_mark: `7.*`: Close Handling
 - :white_large_square: `9.*`: Limits/Performance (_partially out of scope_ — see notes below)
 - :white_check_mark: `10.*`: Misc
 - :white_large_square: `12.*`: WebSocket Compression/Payloads (_not yet implemented_)
 - :white_large_square: `13.*`: WebSocket Compression/Parameters (_not yet implemented_)

### Exclusions

 - `1.1.6-8`, `1.2.6-8`: Frame size tests.
    > **NOTES**:
    >
    > The principle focus here is testing the default configuration.
    > These tests exceed the default maximum frame size.
    >
    > (You can always run `./configure` with a larger frame size and
    > remove these from the exclusions list in
    > [`fuzzingclient.json`](./config/fuzzingclient.json)).
 - `2.10,11`: Not required by spec.
 - `6.3,4,9,1-21`: Out of scope.
    > **NOTES**: Yimmo doesn't perform content-encoding verification.
    >
    > If your service or one of its clients invalid transmit invalid UTF-8,
    > Yimmo will happily relay it for you.
 - `9.*`: Limits
    > **NOTES**: This series is testing limits which will pass/fail
    > based on values set when ``./configure`` is run.
    >
    > The principle focus here is testing the default configuration.
    >
    > (You can always run `./configure` with a larger frame size and
    > remove these from the exclusions list in
    > [`fuzzingclient.json`](./config/fuzzingclient.json)).
 - `12,13`: Libyimmo does not yet implement websockets compression.


## Running Locally

### Docker Compose

Build a test image by invoking [`build-yimmo-test-image.sh`](../build-yimmo-test-image.sh).

Run the test using docker compose:

```bash
# In this directory:
docker compose up --abort-on-container-exit
```

### Local Test


 1. In your build directory: `make && ./ci/ws/yimmo-ws-echo`
 2. In this directory: `./autobahn-local.sh`

