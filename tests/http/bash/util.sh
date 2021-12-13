#!/usr/bin/env bash
#===============================================================================
#
# utility: Generic utility functions
#
#-------------------------------------------------------------------------------

CURL="${CURL:-"$( type -p curl )"}"
JQ="${JQ:-"$( type -p jq )"}"

#--------------------------------------
#       Shell Characteristics:
#--------------------------------------

shell_is_interactive() {
    case "$-" in
        *i*)
            return 0;
            ;;
        *)
            return 1;
            ;;
    esac
}

stdout_is_terminal() {
    if [ -t 1 ]; then
        return 0;
    else
        return 1;
    fi
}

stderr_is_terminal() {
    if [ -t 2 ]; then
        return 0;
    else
        return 1;
    fi
}


#--------------------------------------
#              Verbose:
#--------------------------------------

# No-op function
dry_run() {
    printf '\e[00;02;34mSKIP: %s\e[00m\n' "$@" >&2
}

# Print input args to STDERR and exit 1
err_bail() {
    printf '\e[00;31mFATAL: %s\e[00m\n' "$@" >&2
    exit 1
}

# Log to stderr
log_debug() {
    printf '\e[00;36mDEBUG: \e[00;36m%s\e[00m\n' "$@" >&2
}

log_info() {
    printf '\e[00;32mINFO: \e[00m%s\e[00m\n' "$@" >&2
}

log_warn() {
    printf '\e[00;33mWARN: %s\e[00m\n' "$@" >&2
}

log_error() {
    printf '\e[00;31mERROR: %s\e[00m\n' "$@" >&2
}

log_src_debug() {
    printf '\e[00;02m%s \e[00;36mDEBUG: %s\e[00m\n' \
        "${BASH_SOURCE[1]}:${FUNCNAME[1]}:${BASH_LINENO[0]}" "$@" >&2
}

log_src_info() {
    printf '\e[00;02m%s \e[00;32mINFO: \e[00m%s\e[00m\n' \
        "${BASH_SOURCE[1]}:${FUNCNAME[1]}:${BASH_LINENO[0]}" "$@" >&2
}

log_src_warn() {
    printf '\e[00;02m%s \e[00;33mWARN: %s\e[00m\n' \
        "${BASH_SOURCE[1]}:${FUNCNAME[1]}:${BASH_LINENO[0]}" "$@" >&2
}

log_src_error() {
    printf '\e[00;02m%s \e[00;01;31mERROR: %s\e[00m\n' \
        "${BASH_SOURCE[1]}:${FUNCNAME[1]}:${BASH_LINENO[0]}" "$@" >&2
}

# Print the value of a variable with name label.
# $1     - Variable name as a string
print_var() {
    local var_name
    var_name="$1"
    printf "\e[00;34m%s\e[00m=\"\e[00;33m%s\e[00m\"\n" "${var_name}" "${!var_name}"
}

# Run a command, logging the path and arguments
run_cmd() {
    local cmd
    cmd="$1" ; shift
    echo -e "\e[00;02mEXEC: \e[00;32m${cmd} $@\e[00m" >&2
    ${DRY} ${cmd} "$@"
}

hr() {
    local hdr_char
    hdr_char="$1" ; shift
    printf "\n%*s\n" "${COLUMNS:-$( tput cols )}" '' \
        | tr ' ' "${hdr_char[0]}"
}


#===============================================================================
#
# CURL TESTS:
#
#-------------------------------------------------------------------------------

REQ_CURL_MAJOR="7"
REQ_CURL_MINOR="70"

DEFAULT_CURL_OPTS=(
    "-s"
    "-S"
    "--connect-timeout" "1.0"
    "--fail-early"
    "--max-time" "2.0"
)

CURL_OPTS="${CURL_OPTS:-"${DEFAULT_CURL_OPTS[@]}"}"

curl_or_die() {
    if [ -z "${CURL}" ]; then
        err_bail "CURL not found (try setting the CURL env var)"
    fi

    local v_str v_info
    v_str="$( "${CURL}" -V | awk 'NR==1{print $2}')"
    v_info=($(echo ${v_str//./ }))

    if [ -z "${v_str}" -o "${#v_info[@]}" -lt 2 ]; then
        err_bail "Unable to determine curl version: \"${v_str}\""
    fi

    if [ ${v_info[1]} -lt ${REQ_CURL_MINOR} -a ${v_info[0]} -lt ${REQ_CURL_MAJOR} ]; then
        err_bail "curl version is ${v_str}. Require >= ${REQ_CURL_MAJOR}.${REQ_CURL_MINOR}"
    fi

    log_info "CURL version: ${v_str}"
}


jq_or_die() {
    if [ -z "${JQ}" ]; then
        err_bail "JQ not found (try setting the JQ env var)"
    fi

    log_info "JQ version: $( jq -V )"
}

ctest() {
    local c_result_json r_val

    r_val=0
    c_result_json="$(mktemp )"

    run_cmd "${CURL}" \
        ${CURL_OPTS} \
        -o /dev/null \
        -w '%{json}' \
        "$@" >"${c_result_json}"

    if [[ $? != 0 ]]; then
        return 1
    fi

    if [ -n "${EXPECT_STATUS}" ]; then
        ctest_status "${c_result_json}" "${EXPECT_STATUS}" || r_val=1
    fi

    rm -f "${c_result_json}"

    return ${r_val}
}

ctest_status() {
    local c_status

    if [[ -z "$1" ]] || [[ -z "$2" ]]; then
        log_error "Expected two args. Got $#."
        return 1
    fi

    c_status="$( cat "$1" | "${JQ}" -r '.http_code')"

    if [[ ${c_status} != ${2} ]]; then
        log_error "Expected status: \"$2\". Got: \"${c_status}\""
        return 1
    fi

    echo "${c_status}"
    return 0
}

