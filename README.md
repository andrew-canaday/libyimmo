# libyimmo

> :warning: **WARNING**: _This is my long running, spare time, for fun,
> project._ Most of it works pretty well (it's [pretty _fast!_](#gisty-benchmarks)), but it also
> bears the marks of being my on-again/off-again, personal C-refresher/protocol
> exploration playground...
>
> To be clear: I wouldn't use this in production until after some test coverage
> and usability things have been addressed.
> (I generally use [gunicorn](https://gunicorn.org/) for my production WSGI workloads and
> [libwebsockets](https://corewebsockets.org/) for websockets. Check 'em out!)
>
> There's a bit of cleanup work in flight, and _things are likely to change a
> bit as the refactor proceeds!_
>
> :information_source: **I'm not accepting PR's, at the moment**, due to the
> above and some procedural things to resolve re: code contributions.
>
> _Please feel free to have a peek! It's **GPL'd**, so you are free to fork
> it and hack around!_
>
> (Just mind the cobwebs).

# Introduction
Yimmo (<i><b>Y</b>up, <b>I</b> <b>M</b>ade <b>M</b>y <b>O</b>wn</i>) is a
streaming socket framework. The I/O architecture is a traditional reactor
pattern which uses [libev](http://software.schmorp.de/pkg/coreev.html) for I/O readiness notifications.

> :bulb: Want to get the gist in a hurry?
>
> - [HTTP](./example/doc/HTTP.md) — minimal libyimmo_http web server
> - [WebSockets](./example/doc/WS.md) — minimal libyimmo_ws socket server
> - [WSGI](http://blog.yimmo.org/yimmo/wsgi/index.html)

## License

This project is licensed under version 3 of the GNU GPL (see [LICENSE](./LICENSE)).

> :information_source: **NOTE**:
>
> _Some_ auxiliary files may be licensed under more
> permissive licenses (e.g. the [example](./example) sources are licensed under the
> MIT/expat license).
>
> Except where otherwise explicitly stated, the files contained herein are
> subject to the terms defined by the GNU General Public License v3, as
> published by the Free Software Foundation.

## Documentation

 - [libyimmo](http://blog.yimmo.org/yimmo/index.html)
 - [WSGI](http://blog.yimmo.org/yimmo/wsgi/index.html)

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

 - Compiler: Apple Clang 12.0.5
 - CFLAGS: `-Ofast`
 - Hardware (compile and test):  Mac Mini (M1, 16GB RAM).
 - Tested using: [Apache Bench](https://httpd.apache.org/docs/2.4/programs/ab.html).

### Apache Bench, 250k clients, 10k HTML Payload (WSGI)

> (:wave: **Reminder**: the overwhelming odds are that _your Python WSGI server
> is not the bottleneck in your production workloads!_)

```
Document Path:          /10k
Document Length:        10240 bytes

Concurrency Level:      32
Time taken for tests:   5.717 seconds
Complete requests:      250000
Failed requests:        0
Keep-Alive requests:    250000
Total transferred:      2586500000 bytes
HTML transferred:       2560000000 bytes
Requests per second:    43729.90 [#/sec] (mean)
Time per request:       0.732 [ms] (mean)
Time per request:       0.023 [ms] (mean, across all concurrent requests)
Transfer rate:          441825.75 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       1
Processing:     0    1   2.2      1     127
Waiting:        0    1   2.2      1     127
Total:          0    1   2.2      1     127
```

### Apache Bench, 2M clients, ..2 byte status payload.. (C Example Server)

(This one is a little silly. The only payload hooked up to the HTTP example
is a two-byte "OK" status endpoint...)

```
Document Path:          /status
Document Length:        2 bytes

Concurrency Level:      32
Time taken for tests:   8.904 seconds
Complete requests:      2000000
Failed requests:        0
Keep-Alive requests:    2000000
Total transferred:      178000000 bytes
HTML transferred:       4000000 bytes
Requests per second:    224613.31 [#/sec] (mean)
Time per request:       0.142 [ms] (mean)
Time per request:       0.004 [ms] (mean, across all concurrent requests)
Transfer rate:          19522.05 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       1
Processing:     0    0   0.0      0       1
Waiting:        0    0   0.0      0       1
Total:          0    0   0.0      0       2
```

