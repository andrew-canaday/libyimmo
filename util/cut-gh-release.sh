#!/usr/bin/env bash
#===============================================================================
#
# NOTICE: THIS AUXILIARY FILE IS LICENSED USING THE MIT LICENSE.
#
# Copyright (c) 2021 Andrew T. Canaday
# This file is licensed under the MIT license.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
#-------------------------------------------------------------------------------


RELEASE_GH_TOKEN="$1"; shift
RELEASE_GH_REPOSITORY="$1"; shift
RELEASE_GH_REPO_NAME="${RELEASE_GH_REPOSITORY##*/}"
RELEASE_GH_REF_NAME="$1"; shift
RELEASE_GH_SHA="$1"; shift
RELEASE_PROJECT_NAME="${1:-${RELEASE_GH_REPO_NAME}}" ; shift

RELEASE_JSON="$( mktemp "release-json-XXXXXX" )"

log_info() {
    printf '(\033[00;02;m%s\033[00;35;m)\nINFO\033[00;m: %s\n' \
        "${BASH_SOURCE[1]##*/}:${FUNCNAME[1]}:${BASH_LINENO[0]}" "$@" >&2
}

log_info "RELEASE_GH_REPOSITORY: ${RELEASE_GH_REPOSITORY}"
log_info "RELEASE_GH_REPO_NAME:  ${RELEASE_GH_REPO_NAME}"
log_info "RELEASE_GH_REF_NAME:   ${RELEASE_GH_REF_NAME}"
log_info "RELEASE_GH_SHA:        ${RELEASE_GH_SHA}"
log_info "RELEASE_PROJECT_NAME:  ${RELEASE_PROJECT_NAME}"

create_release() {
    local release_body payload_json
    payload_json="$( mktemp "release_body-XXXXXX" )"

    pushd ${0%/*}
    release_body="$( git tag ${RELEASE_GH_REF_NAME}  -n1000 \
        | tail +3 \
        | sed 's/^    //g' \
        | jq -R -s .
        )"
    popd

    # TODO: use jq or something...
    echo -n '{
          "accept": "application/vnd.github.v3+json",
          "tag_name":"'${RELEASE_GH_REF_NAME}'",
          "target_commitish": "'${RELEASE_GH_SHA}'",
          "body": '${release_body}'
          }' | jq -c > ${payload_json}

    log_info "Release body:"
    cat ${payload_json} | jq '.'

    curl \
      -X POST \
      -f \
      -H 'Accept: application/vnd.github.v3+json' \
      -H "authorization: Bearer $RELEASE_GH_TOKEN" \
      -H 'content-type: application/json' \
      https://api.github.com/repos/$RELEASE_GH_REPOSITORY/releases \
      --data-binary @${payload_json} \
      -o ${RELEASE_JSON}
}

upload_dist_tar_gz() {
    local gh_upload_url upload_url asset_url dist_file

    dist_file=${RELEASE_PROJECT_NAME}-${RELEASE_GH_REF_NAME}.tar.gz
    gh_upload_url="$( cat ${RELEASE_JSON} | jq -r '.upload_url' )"
    upload_url="${gh_upload_url%%\{*}"
    asset_url="${upload_url}?name=${dist_file}&label=${dist_file}"

    log_info "gh_upload_url: ${gh_upload_url}"
    log_info "upload_url:    ${upload_url}"
    log_info "asset_url:     ${asset_url}"
    curl \
      -X POST \
      -f \
      -H "Accept: application/vnd.github.v3+json" \
      -H "authorization: Bearer ${RELEASE_GH_TOKEN}" \
      -H 'Content-Type: application/gzip' \
      --url ${asset_url} \
      -T ./${dist_file}
}

create_release && upload_dist_tar_gz

