#!/usr/bin/env bash

__thispath="$0"
__thiscmd="${__thispath##*/}"
__thisdir="${__thispath%/*}"

export YMO_NO_CLIENTS=100
export YMO_NO_CONCURRENT=${YMO_NO_CONCURRENT:-"4"}
${__thisdir}/run-ab.sh
