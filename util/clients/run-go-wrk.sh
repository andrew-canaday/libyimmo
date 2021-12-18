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

