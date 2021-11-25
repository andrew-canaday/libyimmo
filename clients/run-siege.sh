#!/usr/bin/env bash

YMO_EXAMPLE_URL="${YMO_EXAMPLE_URL:-'http://127.0.0.1:8081/index.html'}"
siege \
    -c 40 \
    --time=1m \
    -b - \
    --no-parser \
    "${YMO_EXAMPLE_URL}"

# EOF

