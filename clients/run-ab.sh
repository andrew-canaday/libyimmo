#!/usr/bin/env bash

function run_cmd() {
    local cmd
    cmd="$1" ; shift
    echo -e "\e[00;02mEXEC: \e[00;32m${cmd} $@\e[00m" >&2
    ${DRY} ${cmd} "$@"
}

if [ -z ${YMO_NO_KEEPALIVE} ]; then
    YMO_KEEPALIVE="-k"
fi

run_cmd ab \
    ${YMO_KEEPALIVE} \
    ${YMO_AB_VERBOSITY} \
    -r \
    -n ${YMO_NO_CLIENTS:-"100000"} \
    -c ${YMO_NO_CONCURRENT:-"32"} \
    "${YMO_EXAMPLE_URL:-"http://127.0.0.1:8081/status"}"

# EOF

