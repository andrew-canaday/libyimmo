#!/usr/bin/env bash

__thisdir="${0%/*}"
__repodir="${__thisdir%/*}"

find "${__repodir}" \
    \( -iname "*.c" -or -iname "*.h" \) \
    -exec uncrustify -c ${__repodir}/uncrustify.cfg \
    --no-backup '{}' \+


