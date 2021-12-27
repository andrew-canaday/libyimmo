#!/usr/bin/env bash

IMAGE_NAME="${IMAGE_NAME:-"yimmo-ci-test:local"}"

. ${0%/*}/../util/bash/ymo-utils.sh

log_info "Changing to directory \"${0%/*}/../\""
cd ${0%/*}/../


log_info "Building docker image"
docker build \
  -t "${IMAGE_NAME}" \
  -f ./docker/Dockerfile \
  --build-arg "OCI_IMAGE_SOURCE=${OCI_IMAGE_SOURCE:-"none"}" \
  --target yimmo-test-http \
  . && log_info "Done"

