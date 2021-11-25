#!/usr/bin/env bash
__thispath="$0"
__thiscmd="${__thispath##*/}"
__thisdir="${__thispath%/*}"

# Usage: go-wrk <options> <url>
# Options:
#   -H      Header to add to each request (you can define multiple -H flags) (Default )
#   -M      HTTP method (Default GET)
#   -T      Socket/request timeout in ms (Default 1000)
#   -body   request body string or #name (Default )
#   -c      Number of goroutines to use (concurrent connections) (Default 10)
#   -ca     CA file to verify peer against (SSL/TLS) (Default )
#   -cert   CA certificate file to verify peer against (SSL/TLS) (Default )
#   -d      Duration of test in seconds (Default 10)
#   -f      Playback file name (Default <empty>)
#   -help   Print help (Default false)
#   -host   Host Header (Default )
#   -http   Use HTTP/2 (Default true)
#   -key    Private key file name (SSL/TLS (Default )
#   -no-c   Disable Compression - Prevents sending the "Accept-Encoding: gzip" header (Default false)
#   -no-ka  Disable KeepAlive - prevents re-use of TCP connections between different HTTP requests (Default false)
#   -no-vr  Skip verifying SSL certificate of the server (Default false)
#   -redir  Allow Redirects (Default false)
#   -v      Print version details (Default false)

function run_cmd() {
    local cmd
    cmd="$1" ; shift
    echo -e "\e[00;02mEXEC: \e[00;32m${cmd} $@\e[00m" >&2
    ${DRY} ${cmd} "$@"
}

export YMO_TEST_DURATION="${YMO_TEST_DURATION:-"10"}"
export YMO_NO_CLIENTS="${YMO_NO_CLIENTS:-"50000"}"
export YMO_NO_CONCURRENT="${YMO_NO_CONCURRENT:-"100"}"
export YMO_EXAMPLE_URL="${YMO_EXAMPLE_URL:-"http://localhost:8081/index.html"}"

run_cmd go-wrk \
    -http=True \
    -c ${YMO_NO_CONCURRENT} \
    -d ${YMO_TEST_DURATION} \
    "${YMO_EXAMPLE_URL}"

# EOF

