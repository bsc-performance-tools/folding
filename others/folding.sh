#!/bin/bash

SCRIPT_PATH=`readlink -f $0`
FOLDING_HOME=`dirname ${SCRIPT_PATH}`
if [[ ! -d ${FOLDING_HOME} ]] ; then
	echo "Cannot locate FOLDING_HOME (should be ${FOLDING_HOME} ?)"
	exit
else
	export FOLDING_HOME=${FOLDING_HOME%/bin}
fi

export LD_LIBRARY_PATH=${FOLDING_HOME}/lib

#
# Process optional parameters first
#

function process_model ()
{
	local model="$1"
	local basename_model=`basename ${model}`
	local result=""

	if [[ -f "${basename_model}" ]] ; then
		# Given model file in current directory
		result="-model ${PWD}/${basename_model} "
	elif [[ -f "${1}" ]] ; then
		# Given model file in current directory
		result="-model ${1} "
	elif [[ -d "${basename_model}" ]] ; then
		# Given model directory in current directory
		for file in "${basename_model}"/*xml
		do
			result+="-model ${PWD}/${file} "
		done
	elif [[ -d "${FOLDING_HOME}/etc/models/${model}" ]] ; then
		# Given model directory in folding installation
		for file in "${FOLDING_HOME}/etc/models/${model}"/*xml
		do
			result+="-model ${file} "
		done
	else
		return 0
	fi

	echo $result
	return 1
}

SHOW_COMMANDS=no
for param in "$@"
do
	if [[ "${1}" == "-model" ]] ; then
		MODELS_SUFFIX+=$(process_model "${2}")
		if [[ $? -ne 1 ]] ; then
			echo "Cannot process model ${2}"
			exit
		fi
		shift
		shift
	elif  [[ "${1}" == "-source" ]] ; then
		echo Given source directory: `readlink -fn ${2}`
		SOURCE_DIRECTORY_SUFFIX="-source `readlink -fn ${2}`"
		shift
		shift
	elif [[ "${1}" == "-region" ]] ; then
		REQUESTED_REGIONS_TO_FOLD+=" -region ${2}"
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
	echo "Usage: ${0} [-model M] [-source S] [-region R] Trace InstanceSeparator/SemanticSeparator"
	echo ""
	echo "            -model M         : Uses performance model M when generating plots"
	echo "                               Available models in $FOLDING_HOME/etc/models:"
	echo -n "                               "
	MODELS=`echo ${FOLDING_HOME}/etc/models/*`
	for m in ${MODELS}
	do
		echo -n `basename ${m}`
		echo -n " "
	done
	echo ""
	echo ""
	echo "            -source S        : Indicates where the source code of the application is located"
	echo "            -region R        : Requests to apply the folding to region R"
	echo "            Trace            : Paraver trace-file"
	echo "            InstanceSeparator: Label or value of the event type to separate instances within tracefile"
	echo "            SemanticSeparator: .csv file generated from Paraver to separate instances within tracefile"
	exit
fi

if [[ ! -f ${1} ]] ; then
	echo "Cannot access tracefile ${1}"
	exit
else
	PRVDIR=`readlink -f ${1}`
	PRVDIR=`dirname ${PRVDIR}`
	PRVFILE=${1}
	PRVBASE=`basename ${PRVFILE} .prv`
	PCFFILE=${PRVDIR}/${PRVBASE}.pcf
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

if [[ -z ${REQUESTED_REGIONS_TO_FOLD+x} ]] ; then
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
else
	EXTRA_INTERPOLATE_FLAGS=${REQUESTED_REGIONS_TO_FOLD}
fi

# BASENAME_PRV="${1%.*}"
BASENAME_PRV=${PRVBASE}

mkdir -p ${BASENAME_PRV} || exit
cd ${BASENAME_PRV} || exit

if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
	echo Executing: ${FOLDING_HOME}/bin/codeblocks ${SOURCE_DIRECTORY_SUFFIX} \"${PRVDIR}/${BASENAME_PRV}.prv\"
fi
${FOLDING_HOME}/bin/codeblocks ${SOURCE_DIRECTORY_SUFFIX} "${PRVDIR}/${BASENAME_PRV}.prv" || exit

if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
	echo Executing: ${FOLDING_HOME}/bin/fuse \"${BASENAME_PRV}.codeblocks.prv\"
fi
${FOLDING_HOME}/bin/fuse "${BASENAME_PRV}.codeblocks.prv" || exit

if [[ "${BASENAME_CSV}" = "" ]] ; then
	if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
		echo ${FOLDING_HOME}/bin/extract -separator \"${2}\" \"${BASENAME_PRV}.codeblocks.fused.prv\"
	fi
	${FOLDING_HOME}/bin/extract -separator "${2}" "${BASENAME_PRV}.codeblocks.fused.prv" || exit
else
	if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
		echo ${FOLDING_HOME}/bin/extract -semantic \"${BASENAME_CSV}.csv\" \"${BASENAME_PRV}.codeblocks.fused.prv\"
	fi
	${FOLDING_HOME}/bin/extract -semantic "${BASENAME_CSV}.csv" "${BASENAME_PRV}.codeblocks.fused.prv" || exit
fi

if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
	echo ${FOLDING_HOME}/bin/interpolate -max-samples-distance 2000 -sigma-times 2.0 -use-median -feed-first-occurrence any ${EXTRA_INTERPOLATE_FLAGS} ${MODELS_SUFFIX} ${SOURCE_DIRECTORY_SUFFIX} "${BASENAME_PRV}.codeblocks.fused.extract"
fi

# Hook for Paraver
echo Output directory: ${PWD}

${FOLDING_HOME}/bin/interpolate \
 -max-samples-distance 1500 \
 -sigma-times 1.5 \
 -use-median \
 -feed-first-occurrence any \
 -callstack-processor pct 15 5.0 \
 ${EXTRA_INTERPOLATE_FLAGS} \
 ${MODELS_SUFFIX} \
 ${SOURCE_DIRECTORY_SUFFIX} \
 "${BASENAME_PRV}.codeblocks.fused.extract"

cd ..

