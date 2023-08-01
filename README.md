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
> [libwebsockets](https://libwebsockets.org/) for websockets.** Check 'em out!_
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
 - [libyaml](https://pyyaml.org/wiki/LibYAML) for [optional?] yaml config parsing

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
### Gisty Benchmarks (Moved)

[Here](./doc/GISTY_BENCHMARKS.md)
