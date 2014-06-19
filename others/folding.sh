#!/bin/bash

SCRIPT_PATH=`readlink -f $0`
FOLDING_HOME=`dirname ${SCRIPT_PATH}`
if [[ ! -d ${FOLDING_HOME} ]] ; then
	echo "Cannot locate FOLDING_HOME (should be ${FOLDING_HOME} ?)"
	exit
else
	export FOLDING_HOME=${FOLDING_HOME%/bin}
fi

LD_LIBRARY_PATH=${FOLDING_HOME}/lib

#
# Process optional parameters first
#

SHOW_COMMANDS=no
for param in "$@"
do
	if [[ "${1}" == "-model" ]] ; then
		BASENAME_MODEL=`basename ${2}`
		if [[ "${2}"  == "${BASENAME_MODEL}" ]] ; then
			if [[ -f "${2}" ]]; then
				FILENAME_MODEL=${2}
			elif [[ -f ${FOLDING_HOME}/etc/models/${2} ]] ; then
				FILENAME_MODEL=${FOLDING_HOME}/etc/models/${2}
			else
				echo "Cannot find model ${2}"
				exit
			fi
		else
			if [[ -f "${2}" ]]; then
				FILENAME_MODEL=${2}
			else
				echo "Cannot find model ${2}"
				exit
			fi
		fi
		echo Given model: ${FILENAME_MODEL}
		MODELS_SUFFIX+="-model ${FILENAME_MODEL} "
		shift
		shift
	elif  [[ "${1}" == "-source" ]] ; then
		echo Given source directory: `readlink -fn ${2}`
		SOURCE_DIRECTORY_SUFFIX="-source `readlink -fn ${2}`"
		shift
		shift
	elif [[ "${1}" == "-show-commands" ]] ; then
		SHOW_COMMANDS=yes
		shift
	else
		break
	fi
done

#
# Check for mandatory parameters
#

if [[ $# -ne 2 ]] ; then
	echo "Usage: ${0} [-model M] [-source S] Trace InstanceSeparator/SemanticSeparator"
	echo ""
	echo "            -model M         : Use performance model M when generating plots (see $FOLDING_HOME/others/models)"
	echo "            -source S        : Indicate where the source code of the application is located"
	echo "            Trace            : Paraver trace-file"
	echo "            InstanceSeparator: Label or value f the event type to separate instances within tracefile"
	echo "            SemanticSeparator: .csv file generated from Paraver to separate instances within tracefile"
	exit
fi

if [[ ! -f ${1} ]] ; then
	echo "Cannot access tracefile ${1}"
	exit
else
	PRVBASE=`basename ${1} .prv`
	PRVFILE=${1}
	PCFFILE=`dirname ${PRVFILE}`/${PRVBASE}.pcf
fi

EXTENSION_PRV="${1##*.}"
if [[ "${EXTENSION_PRV}" != "prv" ]] ; then
	echo "Invalid extension for a Paraver tracefile (file was ${1})"
	exit
fi
if test ! -f ${PCFFILE}; then
	echo "Cannot access PCF file for tracefile ${1}"
	exit
fi
NUMERICAL_RE='^[0-9]+$'

if [[ ${2} =~ ${NUMERICAL_RE} ]]; then
	echo "Using event type ${2} as instance delimiter"
	BASENAME_CSV=""
	if [[ ${2} = 90000001 ]] ; then
		 EXTRA_INTERPOLATE_FLAGS+=" -region-start-with Cluster_"
	fi
else
	BASENAME_CSV=""
	EXTENSION_CSV="${2##*.}"
	if [[ "${EXTENSION_CSV}" = "csv" ]] ; then
		if [[ -r ${2} ]]; then
			BASENAME_CSV="${2%.*}"
			echo "Using semantic file ${2} as instance delimiter"
		fi
	fi

	if [[ "${BASENAME_CSV}" = "" ]] ; then
		echo "Using event type with name '${2}' as instance delimiter"
		if [[ "${2}" = "Cluster ID" ]] ; then
			 EXTRA_INTERPOLATE_FLAGS+=" -region-start-with Cluster_"
		fi
	fi
fi

BASENAME_PRV="${1%.*}"

mkdir -p ${BASENAME_PRV}
cd ${BASENAME_PRV}

if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
	echo Executing: ${FOLDING_HOME}/bin/codeblocks ${SOURCE_DIRECTORY_SUFFIX} \"../${BASENAME_PRV}.prv\"
fi
${FOLDING_HOME}/bin/codeblocks ${SOURCE_DIRECTORY_SUFFIX} "../${BASENAME_PRV}.prv" || exit 1

if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
	echo Executing: ${FOLDING_HOME}/bin/fuse \"${BASENAME_PRV}.codeblocks.prv\"
fi
${FOLDING_HOME}/bin/fuse "${BASENAME_PRV}.codeblocks.prv" || exit 1

if [[ "${BASENAME_CSV}" = "" ]] ; then
	if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
		echo ${FOLDING_HOME}/bin/extract -separator \"${2}\" \"${BASENAME_PRV}.codeblocks.fused.prv\"
	fi
	${FOLDING_HOME}/bin/extract -separator "${2}" "${BASENAME_PRV}.codeblocks.fused.prv" || exit 1
else
	if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
		echo ${FOLDING_HOME}/bin/extract -semantic \"${BASENAME_CSV}.csv\" \"${BASENAME_PRV}.codeblocks.fused.prv\"
	fi
	${FOLDING_HOME}/bin/extract -semantic "${BASENAME_CSV}.csv" "${BASENAME_PRV}.codeblocks.fused.prv" || exit 1
fi

if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
	echo ${FOLDING_HOME}/bin/interpolate -max-samples-distance 2000 -sigma-times 2.0 -use-median -feed-first-occurrence any ${EXTRA_INTERPOLATE_FLAGS} ${MODELS_SUFFIX} ${SOURCE_DIRECTORY_SUFFIX} "${BASENAME_PRV}.codeblocks.fused.extract"
fi

${FOLDING_HOME}/bin/interpolate \
 -max-samples-distance 2000 \
 -sigma-times 2.0 \
 -use-median \
 -feed-first-occurrence any \
 ${EXTRA_INTERPOLATE_FLAGS} \
 ${MODELS_SUFFIX} \
 ${SOURCE_DIRECTORY_SUFFIX} \
 "${BASENAME_PRV}.codeblocks.fused.extract"

cd ..

