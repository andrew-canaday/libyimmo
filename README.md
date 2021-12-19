# libyimmo

[![Configure, Make, and Check (Defaults + WSGI Build)](https://github.com/andrew-canaday/libyimmo/actions/workflows/configure_make_check.yml/badge.svg)](https://github.com/andrew-canaday/libyimmo/actions/workflows/configure_make_check.yml)
[![Server Tests](https://github.com/andrew-canaday/libyimmo/actions/workflows/server-tests.yml/badge.svg)](https://github.com/andrew-canaday/libyimmo/actions/workflows/server-tests.yml)
[![Publish Yimmo-WSGI Docker Image](https://github.com/andrew-canaday/libyimmo/actions/workflows/docker_wsgi.yml/badge.svg)](https://github.com/andrew-canaday/libyimmo/actions/workflows/docker_wsgi.yml)
[![Create Release on Tag](https://github.com/andrew-canaday/libyimmo/actions/workflows/create_release_on_tag.yml/badge.svg)](https://github.com/andrew-canaday/libyimmo/actions/workflows/create_release_on_tag.yml)

> :warning: **WARNING**: _This is my long running, spare time, for fun,
> project._ Most of it works pretty well (it's [pretty _fast!_](#gisty-benchmarks)), but it also
> bears the marks of being my on-again/off-again, personal C-refresher/protocol
> exploration playground...
>
> To be clear: I wouldn't use this in production until after some test coverage
> and usability things have been addressed.
> (_**I use [gunicorn](https://gunicorn.org/) for my production WSGI workloads and
> [libwebsockets](https://corewebsockets.org/) for websockets.** Check 'em out!_)
>
> There's a bit of cleanup work in flight, and _things are likely to change a
> bit as the refactor proceeds!_
>
> <hr />
>
> :hand: **I'm not accepting PR's, at the moment**, due to the
> above and some procedural things to resolve re: code contributions.
>
> _Please feel free to have a peek! It's **GPL'd**, so you are free to fork
> it and hack around!_
>
> (Just mind the cobwebs).
>
> <hr />
>
> :thumbsup: **Issues are welcome, though!** If you're tinkering with the
> library and discover a bug, have a feature request, or want to surface
> a usability issue, please feel free! (Documentation issues are
> also welcome, though the odds are high it's already on one TODO list or
> another).
>
> **Bonus points**: if you add a [label](https://github.com/andrew-canaday/libyimmo/labels).

# Introduction
Yimmo (<i><b>Y</b>up, <b>I</b> <b>M</b>ade <b>M</b>y <b>O</b>wn</i>) is a
streaming socket framework. The I/O architecture is a traditional reactor
pattern which uses [libev](http://software.schmorp.de/pkg/coreev.html) for I/O readiness notifications.

> :bulb: Want to get the gist in a hurry?
>
> - [HTTP](./examples/doc/HTTP.md) — minimal libyimmo_http web server
> - [WebSockets](./examples/doc/WS.md) — minimal libyimmo_ws socket server
> - [WSGI](http://blog.yimmo.org/yimmo/wsgi/index.html)

### (Docker Quickstart)

Don't want to build it? Try the WSGI image:

```bash
# This will start the WSGI server on port 8081:
docker run \
    -v ${PWD}/wsgi/demo:/opt/yimmo/demo \
    -p 8081:8081 \
    -e PYTHONPATH="/opt/yimmo/demo" \
    -e YIMMO_WSGI_MODULE="demo_app" \
    -e YIMMO_WSGI_APP="app" \
    andrewcanaday/yimmo-wsgi:0-alpha
```

> **NOTES**:
>
>  - The max HTTP POST body (`4096`) is hardcoded (_sorry! On the TODO list._)
>  - TLS support is _disabled in the image at compile time_.
>  - See [the WSGI docs](http://blog.yimmo.org/yimmo/wsgi/index.html) for more info.
>  - Docker image is currently built for `x86` on Ubuntu
>  - Apple M1 users: the docker image *will run*, but you'll get _way better performance_ building and installing natively (arm64 build on the TODO list).

## License

This project is licensed under version 3 of the GNU GPL (see [LICENSE](./LICENSE)).

> :information_source: **NOTES**:
>
> **Many auxiliary files are licensed under more permissive licenses**, e.g.:
>  - scripts, Dockerfiles, and silly things like that (MIT)
>  - example code (MIT)
>
> Licenses *should* be declared pretty explicitly, but I'm sure I missed some
> (also on the TODO list...along with some *specific* GPL exemption
> declarations — this should all be in place for the `0.1.0` release).
>
> Except where otherwise explicitly stated, the files contained herein are
> subject to the terms defined by the GNU General Public License v3, as
> published by the Free Software Foundation.

## Documentation

 - [Yimmo Docs](http://blog.yimmo.org/yimmo/index.html)
 - [WSGI](http://blog.yimmo.org/yimmo/wsgi/index.html)
 - [Yimmo CI Tests](./ci)

## Features

 - [x] HTTP 1.0
 - [x] HTTP 1.1
 - [x] RFC6455 WebSockets
 - [x] WSGI

### Planned/Partial

 - [ ] MQTT (_partial_)
 - [ ] HTTP2 (_WIP_)

## Building

This library is built using C11 (but _attempts_ to accommodate other C standards
using a [autoconf](https://www.gnu.org/software/autoconf/Libyimmo) macros and
compiler extensions).

### Dependencies

Libyimmo requires the following third party libraries:
 - [libev](http://software.schmorp.de/pkg/coreev.html) for event management
 - [libbsat](https://github.com/andrew-canaday/libbsat) for timeout management
 - a UUID library — either linux-ng or OSSP ([ax_check_uuid_api.m4](./m4/ax_check_uuid_api.m4) for details)

### Setup

Before anything else is done, your source directory has to be initialized:

```bash
./autogen.sh
```

### Compiling

Libyimmo compilation follows the standard GNU idiom:

```bash
# To see a list of configuration options:
./configure --help

# PRO TIP: do your builds outside of the project root!
mkdir -p ./build && cd ./build
../configure --prefix=/usr/local && make && make check && make install
```

## Gisty Benchmarks

> :construction: **These are _informal benchmarks and lack rigor_.** (They're
> mostly just for fun. :smile:).
>
> Notes:
>
> - Naturally, I posted the most flattering benchmarks. :stuck_out_tongue_winking_eye:
> - Results from wrk, seige, and go-wrk differ by ~ 5%.
> - I get better numbers with `gcc-10`, worse with `gcc-11` (difference < 1%)
> - RPS on my home Ubuntu Server, Core i7, 32GB RAM are ~ 60% of those below.
>
> Probably, I'll get some _real_ benchmarks put together when I stand up some
> CI. In the interim, here's the gist:

### Apache Bench, 250k clients, 10k HTML Payload (WSGI)

> (:wave: **Reminder**: the overwhelming odds are that _your Python WSGI server
> is not the bottleneck in your production workloads!_)

- Compiler: Apple clang 12.0.5
- CFLAGS: `-O0`
- Hardware (compile and test):  Mac Mini (M1, 16GB RAM).
- Tested using: [Apache Bench](https://httpd.apache.org/docs/2.4/programs/ab.html).
- `YIMMO_WSGI_NO_PROC=4`
- `YIMMO_WSGI_NO_THREADS=1`

```
Document Path:          /10k
Document Length:        10240 bytes

Concurrency Level:      32
Time taken for tests:   5.533 seconds
Complete requests:      250000
Failed requests:        0
Keep-Alive requests:    250000
Total transferred:      2586500000 bytes
HTML transferred:       2560000000 bytes
Requests per second:    45182.78 [#/sec] (mean)
Time per request:       0.708 [ms] (mean)
Time per request:       0.022 [ms] (mean, across all concurrent requests)
Transfer rate:          456504.87 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       1
Processing:     0    1   0.5      1      30
Waiting:        0    1   0.5      1      30
Total:          0    1   0.5      1      30
```

### Apache Bench, 2M clients, ..2 byte status payload.. (C Example Server)

- Compiler: gcc 11.2.0
- CFLAGS: `-Ofast`
- Hardware (compile and test):  Mac Mini (M1, 16GB RAM).
- Tested using: [Apache Bench](https://httpd.apache.org/docs/2.4/programs/ab.html).
-
(This one is a little silly. The only payload hooked up to the HTTP example
is a two-byte "OK" status endpoint... The RSS stays under `3,870` for the
duration of the test, though).

```
Server Hostname:        127.0.0.1
Server Port:            8081

Document Path:          /status
Document Length:        2 bytes

Concurrency Level:      32
Time taken for tests:   8.441 seconds
Complete requests:      2000000
Failed requests:        0
Keep-Alive requests:    2000000
Total transferred:      178000000 bytes
HTML transferred:       4000000 bytes
Requests per second:    236938.75 [#/sec] (mean)
Time per request:       0.135 [ms] (mean)
Time per request:       0.004 [ms] (mean, across all concurrent requests)
Transfer rate:          20593.31 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       1
Processing:     0    0   0.0      0       1
Waiting:        0    0   0.0      0       1
Total:          0    0   0.0      0       2
```

