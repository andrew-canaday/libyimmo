# yimmo-wsgi

Documentation can be found **[here](http://blog.yimmo.org/yimmo/wsgi/index.html)**.

## Usage

```
yimmo-wsgi LATEST
Usage: yimmo-wsgi [OPTIONS] WSGI_MODULE:WSGI_APP

Options:
  --config, -c     : Path to the yimmo-wsgi config yaml
  --log-level, -l  : Yimmo log level
  --port, -P       : Port
  --no-proc, -p    : Number of processes to run
  --no-threads, -t : Number of worker threads per proc
  --help, -h       : Display usage info (this)

Environment Variables:
  YIMMO_LOG_LEVEL           : libyimmo log level
  YIMMO_SERVER_IDLE_TIMEOUT : socket-level idle disconnect timeout
  YIMMO_WSGI_CONFIG         : yimmo wsgi config file path
  YIMMO_WSGI_NO_PROC        : number of yimmo WSGI processes to run
  YIMMO_WSGI_NO_THREADS     : number of worker threads per process
  YIMMO_WSGI_MODULE         : WSGI module (if not provided as arg)
  YIMMO_WSGI_APP            : WSGI app (if not provided as arg)
  YIMMO_TLS_CERT_PATH       : TLS certificate path
  YIMMO_TLS_KEY_PATH        : TLS private key path

Configuration precedence (greatest to least):
  - command line parameters
  - environment variables
  - config files

# Config Example:
---
log_level: INFO
wsgi:
  port: 8081
  tls:
    cert: /path/to/site.crt
    key: /path/to/cert.pem
  no_proc: 2
  no_threads: 2
```

