#!/usr/bin/env bash

YMO_GUNICORN_WORKER="${YMO_GUNICORN_WORKER:-"gthread"}"

gunicorn -w 4 -k ${YMO_GUNICORN_WORKER} -b 0.0.0.0:8081 'my:app'

# EOF

