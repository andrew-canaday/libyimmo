#!/usr/bin/env bash

GLIBTOOLIZE="$( type -p glibtoolize 2>/dev/null )"
if [ -n "${GLIBTOOLIZE_PATH}" ]; then
    export LIBTOOLIZE="${GLIBTOOLIZE}"
fi

autoreconf -vif

date

# EOF

