# libyimmo

[![Configure, Make, and Check](https://github.com/andrew-canaday/libyimmo/actions/workflows/configure_make_check.yml/badge.svg)](https://github.com/andrew-canaday/libyimmo/actions/workflows/configure_make_check.yml)
[![Server Tests](https://github.com/andrew-canaday/libyimmo/actions/workflows/server-tests.yml/badge.svg)](https://github.com/andrew-canaday/libyimmo/actions/workflows/server-tests.yml)
[![Yimmo-WSGI Docker Image](https://github.com/andrew-canaday/libyimmo/actions/workflows/docker_wsgi.yml/badge.svg)](https://github.com/andrew-canaday/libyimmo/actions/workflows/docker_wsgi.yml)
[![Create Release on Tag](https://github.com/andrew-canaday/libyimmo/actions/workflows/create_release_on_tag.yml/badge.svg)](https://github.com/andrew-canaday/libyimmo/actions/workflows/create_release_on_tag.yml)

# Disclaimer

> :warning: **WARNING**: _This is my long running, spare time, for fun,
> project._ Most of it works pretty well (it's [pretty _fast!_](#gisty-benchmarks)), but it also
> bears the marks of being my on-again/off-again, personal C-refresher/protocol
> exploration playground...
>
> :raised_hands: _**I use [gunicorn](https://gunicorn.org/) for my production WSGI workloads and
> [libwebsockets](https://corewebsockets.org/) for websockets.** Check 'em out!_
>
> <hr />
>
> :hand: **I'm not accepting PR's, at the moment** — there's a bit of
> refactor in flight and some procedural things to resolve re:
> code contributions.
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
> - [WSGI](http://blog.yimmo.org/yimmo/wsgi/index.html) (or try the [docker image](https://hub.docker.com/r/andrewcanaday/yimmo-wsgi))

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
Document Path:          /payload/10k
Document Length:        10240 bytes

Concurrency Level:      64
Time taken for tests:   4.703 seconds
Complete requests:      250000
Failed requests:        0
Keep-Alive requests:    250000
Total transferred:      2583000000 bytes
HTML transferred:       2560000000 bytes
Requests per second:    53158.19 [#/sec] (mean)
Time per request:       1.204 [ms] (mean)
Time per request:       0.019 [ms] (mean, across all concurrent requests)
Transfer rate:          536357.85 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       2
Processing:     0    1   0.8      1      33
Waiting:        0    1   0.8      1      33
Total:          0    1   0.8      1      33
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
Document Path:          /status
Document Length:        0 bytes

Concurrency Level:      200
Time taken for tests:   8.027 seconds
Complete requests:      2000000
Failed requests:        1999999
   (Connect: 0, Receive: 36, Length: 1999963, Exceptions: 0)
Keep-Alive requests:    1999964
Total transferred:      179996760 bytes
HTML transferred:       3999928 bytes
Requests per second:    249151.64 [#/sec] (mean)
Time per request:       0.803 [ms] (mean)
Time per request:       0.004 [ms] (mean, across all concurrent requests)
Transfer rate:          21897.70 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0       6
Processing:     0    1   0.2      1       7
Waiting:        0    1   0.2      1       7
Total:          0    1   0.2      1      12
```

