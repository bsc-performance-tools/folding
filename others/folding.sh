#!/bin/bash

FOLDING_HOME=/home/harald/aplic/folding/trunk-address

if [[ $# -ne 2 ]] ; then
	echo "Usage: ${0} Trace InstanceSeparator/SemanticSeparator"
	echo "            Trace            : Paraver trace-file"
	echo "            InstanceSeparator: Value of the event type to separate instances within tracefile"
	echo "            SemanticSeparator: .csv file generated from Paraver to separate instances within tracefile"
	exit
fi

EXTENSION_PRV="${1##*.}"
if [[ "${EXTENSION_PRV}" != "prv" ]] ; then
	echo "Invalid extension for a Paraver tracefile"
	exit
fi
NUMERICAL_RE='^[0-9]+$'

if [[ ${2} =~ ${NUMERICAL_RE} ]]; then
	echo "Using event type ${2} as instance delimiter"
	BASENAME_CSV=""
else
	EXTENSION_CSV="${2##*.}"
	if [[ "${EXTENSION_CSV}" = "csv" ]] ; then
		if [[ -r ${2} ]]; then
			BASENAME_CSV="${2%.*}"
			echo "Using semantic file ${2} as instance delimiter"
		else
			echo "File ${2} does not exist"
		fi
	else
		echo "Invalid extension for a Paraver semantic file"
	fi
fi

BASENAME_PRV="${1%.*}"

mkdir -p ${BASENAME_PRV}
cd ${BASENAME_PRV}

${FOLDING_HOME}/bin/codeblocks "../${BASENAME_PRV}.prv"
${FOLDING_HOME}/bin/fuse "${BASENAME_PRV}.codeblocks.prv"
if [[ x${BASENAME_CSV} = x ]] ; then
	${FOLDING_HOME}/bin/extract -separator ${2} "${BASENAME_PRV}.codeblocks.fused.prv"
else
	${FOLDING_HOME}/bin/extract -semantic "${BASENAME_CSV}.csv" "${BASENAME_PRV}.codeblocks.fused.prv"
fi
${FOLDING_HOME}/bin/interpolate -max-samples-distance 2000 -sigma-times 1.5 -feed-first-occurrence 1.1.1 "${BASENAME_PRV}.codeblocks.fused.extract"

cd ..

