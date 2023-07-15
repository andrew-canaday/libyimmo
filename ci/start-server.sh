#!/usr/bin/env bash

IMAGE_NAME="${IMAGE_NAME:-"yimmo-ci-test:local"}"

docker run \
  --name yimmo-wsgi-server \
  -e YIMMO_LOG_LEVEL=${YIMMO_LOG_LEVEL:-"INFO"} \
  -e YIMMO_WSGI_NO_PROC=2 \
  -e YIMMO_WSGI_NO_THREADS=2 \
  -p 8081:8081 \
  $IMAGE_NAME

# EOF
