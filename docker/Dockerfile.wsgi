FROM ubuntu:latest

# Avoid interactive prompts for TZ:
ENV TZ=US/Eastern
ENV DEBIAN_FRONTEND=noninteractive

# Paths used at runtime:
ENV LD_LIBRARY_PATH=/usr/local/lib

RUN mkdir -p /opt/src/ \
	&& mkdir -p /opt/build/bsat \
	&& mkdir -p /opt/build/yimmo

ADD . /opt/src/libyimmo

RUN apt update \
	&& apt-get install -y \
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
RUN cd /opt/src/libyimmo && ./autogen.sh

RUN cd /opt/build/bsat \
	&& /opt/src/libbsat/configure CFLAGS="-Ofast -flto" LDFLAGS="-flto" \
	&& make \
	&& make check \
	&& make install

RUN cd /opt/build/yimmo \
	&& /opt/src/libyimmo/configure \
		CFLAGS="-Ofast -flto -DYMO_LOG_LEVEL_MAX=2 -DYMO_LOG_LEVEL_DEFAULT=1" \
		LDFLAGS="-flto" \
		YMO_ALLOC_ALLOW_WEAKREF=0 \
		YMO_ALLOC_ALLOW_WEAK=0 \
		YMO_ALLOC_ALLOW_ALIAS=0 \
		YMO_HDR_HASH_ALLOW_WEAK=0 \
		YMO_HTTP_RECV_BUF_SIZE=4096 \
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


ENTRYPOINT ["/usr/local/bin/yimmo-wsgi"]
