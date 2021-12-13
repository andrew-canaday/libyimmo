#!/usr/bin/env bash
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

