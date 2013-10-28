#!/bin/bash

FOLDING_HOME=

if test $# -ne 2 ; then
	echo "You must pass a Paraver tracefile (.prv) and a Paraver semantic file (.csv)"
	exit
fi

EXTENSION_PRV="${1##*.}"
if test "${EXTENSION_PRV}" != "prv" ; then
	echo "Invalid extension for a Paraver tracefile"
	exit
fi
EXTENSION_CSV="${2##*.}"
if test "${EXTENSION_CSV}" != "csv" ; then
	echo "Invalid extension for a Paraver tracefile"
	exit
fi

cd `dirname ${1}`

BASENAME_PRV="${1%.*}"
BASENAME_CSV="${2%.*}"

${FOLDING_HOME}/bin/codeblocks "${BASENAME_PRV}.prv"
${FOLDING_HOME}/bin/fuse "${BASENAME_PRV}.codeblocks.prv"
${FOLDING_HOME}/bin/extract -semantic "${BASENAME_CSV}.csv" "${BASENAME_PRV}.codeblocks.fused.prv"
${FOLDING_HOME}/bin/interpolate -max-samples-distance 2000 -feed-first-occurrence 1.1.1 "${BASENAME_PRV}.codeblocks.fused.extract"

