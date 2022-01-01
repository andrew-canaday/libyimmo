WebSockets CI
=============

WebSockets are tested using the [Autobahn Test Suite](https://github.com/crossbario/autobahn-testsuite) and run inside a
[docker compose app](./docker-compose.yml).

You can see the latest results from main **[here](http://blog.yimmo.org/results-tmp/yimmo-ws-autobahn/)**.

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
 - :white_check_mark: `6.*`: UTF-8 Handling
 - :white_check_mark: `7.*`: Close Handling
 - :white_check_mark: `9.*`: Limits/Performance
 - :white_check_mark: `10.*`: Misc
 - :white_large_square: `12.*`: WebSocket Compression/Payloads (_not yet implemented_)
 - :white_large_square: `13.*`: WebSocket Compression/Parameters (_not yet implemented_)

### Exclusions

 - `2.10,11`: Not required by spec.
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

