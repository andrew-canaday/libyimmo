#!/usr/bin/env bash
#===============================================================================
#
# NOTICE: THIS AUXILIARY FILE IS LICENSED USING THE MIT LICENSE.
#
# Copyright (c) 2014 Andrew T. Canaday
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

#-------------------------------------------------------------------------------
#
# Generate Self Signed SSL Certificates
#
# For usage: run without arguments.
#
#-------------------------------------------------------------------------------

log_info() {
    printf '\e[00;32mINFO: \e[00m%s\e[00m\n' "$@" >&2
}

run_cmd() {
    local cmd
    cmd="$1" ; shift
    echo -e "\e[00;02mEXEC: \e[00;32m${cmd} $@\e[00m" >&2
    ${DRY} ${cmd} "$@"
}

function main() {
    cert_name="${1}" ; shift
    cert_cn="${1:-"$(hostname)"}" ; shift
    cert_exp="${1:-"365"}" ; shift
    org_unit="${1:-"ymo-test"}" ; shift

    if [ -z "${cert_name}" -o -z "${cert_cn}" -o -z "${cert_exp}" -o -z "${org_unit}" ]; then
        err_bail "Error: $0 CERT_NAME [CN (default: $(hostname)] [EXP (default: 365)] [ORG_UNIT (default: \"Personal\")]"
    fi

    cert_subj=""
    cert_subj+="/C=US"
    cert_subj+="/ST=New York"
    cert_subj+="/L=New York"
    cert_subj+="/O=${USER}"
    cert_subj+="/OU=${org_unit}"
    cert_subj+="/CN=${cert_cn}"

    log_info "Generating ${cert_name}.prv.pem/${cert_name}.cert.pem for:"
    log_info "${cert_subj}" | tr "/" "\n"

    # Generate a self-signed certificate (expires in a year):
    run_cmd openssl \
        req -x509 \
        -newkey rsa:4096 \
        -subj "${cert_subj}" \
        -nodes \
        -keyout "${cert_name}.prv.pem" \
        -out "${cert_name}.cert.pem" \
        -days ${cert_exp}
}

main "$@"

# EOF

