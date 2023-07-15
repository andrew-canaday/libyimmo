# syntax=docker/dockerfile:1
#-------------------------------------------------------------------------------
# Dependencies:
#-------------------------------------------------------------------------------
FROM ubuntu:latest AS yimmo-deps
ARG OCI_IMAGE_SOURCE=none

LABEL org.opencontainers.image.source=$OCI_IMAGE_SOURCE

# Avoid interactive prompts for TZ:
ENV TZ=US/Eastern
ENV DEBIAN_FRONTEND=noninteractive

# Paths used at runtime:
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV PYTHONPATH=/opt/src/libyimmo/wsgi/demo

RUN mkdir -p /opt/src/ \
	&& mkdir -p /opt/build/bsat \
	&& mkdir -p /opt/build/yimmo

RUN apt update
RUN apt-get -qq install -y \
	python3

RUN apt-get -qq install -y \
		git \
		build-essential \
		autotools-dev \
		libtool \
		autoconf \
		automake \
		pkg-config \
		libev-dev \
                uuid-dev \
		libssl-dev \
		python3-dev \
		python3-pip

RUN pip3 install flask

RUN cd /opt/src && git clone https://github.com/andrew-canaday/libbsat
RUN cd /opt/src/libbsat && ./autogen.sh
RUN cd /opt/build/bsat \
	&& /opt/src/libbsat/configure \
		--silent \
		--enable-silent-rules \
	&& make install

ADD . /opt/src/libyimmo
RUN cd /opt/src/libyimmo && ./autogen.sh


#-------------------------------------------------------------------------------
# Test base:
#-------------------------------------------------------------------------------
FROM yimmo-deps AS yimmo-test

ENV YIMMO_SERVER_IDLE_TIMEOUT=2.0
RUN cd /opt/build/yimmo \
	&& /opt/src/libyimmo/configure \
		--enable-examples \
		--enable-wsgi \
		CFLAGS="-DYMO_LOG_LEVEL_MAX=6 -DYMO_LOG_LEVEL_DEFAULT=3" \
		YMO_WS_FRAME_MAX=0xffffffffffffffff \
	&& make \
	&& make check \
	&& make install


#-------------------------------------------------------------------------------
# Test Images:
#-------------------------------------------------------------------------------

# HTTP:
FROM yimmo-test AS yimmo-test-http

ENTRYPOINT ["/usr/local/bin/yimmo-wsgi", "demo_app:app"]

# WS:
FROM yimmo-test AS yimmo-test-ws

ENTRYPOINT ["/opt/build/yimmo/ci/ws/yimmo-ws-echo"]


#-------------------------------------------------------------------------------
# WSGI:
#-------------------------------------------------------------------------------
FROM yimmo-deps AS yimmo-wsgi
ENV PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig

RUN cd /opt/build/yimmo \
	&& /opt/src/libyimmo/configure \
		CFLAGS="-Ofast -flto -DYMO_LOG_LEVEL_MAX=2 -DYMO_LOG_LEVEL_DEFAULT=1" \
		LDFLAGS="-flto" \
		YMO_ALLOC_ALLOW_WEAKREF=0 \
		YMO_ALLOC_ALLOW_WEAK=0 \
		YMO_ALLOC_ALLOW_ALIAS=0 \
		YMO_HDR_HASH_ALLOW_WEAK=0 \
		YMO_HTTP_RECV_BUF_SIZE=4096 \
		YMO_HTTP_MAX_BODY=0x1000000 \
		--silent \
		--enable-wsgi \
		--enable-silent-rules \
	&& make \
	&& make check \
	&& make install

RUN rm -r \
	/opt/src/libyimmo \
	/opt/src/libbsat \
	/opt/build/bsat \
	/opt/build/yimmo

RUN apt-get -qq remove \
		git \
		build-essential \
		autotools-dev \
		libtool \
		autoconf \
		automake \
		pkg-config \
		python3-dev \
		python3-pip \
	&& apt-get clean

ENTRYPOINT ["/usr/local/bin/yimmo-wsgi"]
