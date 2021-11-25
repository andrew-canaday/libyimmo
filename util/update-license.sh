#!/usr/bin/env bash
#=====================================================================
#
# Update the license header in the input file.
#
# - $1: number of lines in existing license header
# - $2: input source
#
# Required env vars:
# - LIBYIMMO_ROOT: source root directory (used for ref file)
#
#---------------------------------------------------------------------

err_bail() {
    echo "ERROR: $@" >&1
    exit 1
}

err_usage() {
    echo "USAGE: $0 NO_LINES SOURCE1 [SOURCE2...]"
    err_bail "$@"
}

if [ -z "${LIBYIMMO_ROOT}" ]; then
    err_bail "LIBYIMMO_ROOT not set"
fi

if [ ! -d "${LIBYIMMO_ROOT}" ]; then
    err_bail "LIBYIMMO_ROOT is not a directory"
fi

REF_FILE="$( cd "${LIBYIMMO_ROOT}/ref" ; echo "${PWD}" )/ymo_c_hdr_text.txt"
if [ ! -r "${REF_FILE}" ]; then
    err_bail "Unable to read ${REF_FILE}"
fi

no_lines="$1"; shift
if [[ -z "${no_lines}" ]] || [[ $no_lines < 1 ]]; then
    err_usage "NO_LINES must be an integer >= 1"
fi

t_src="$( mktemp )"
for src in "$@"; do
    tail -n "+${no_lines}" "${src}" > "${t_src}"
    cat "${REF_FILE}" "${t_src}" > "${src}"
done

# EOF

