#!/bin/bash

SCRIPT_PATH=`readlink -f $0`
FOLDING_HOME=`dirname ${SCRIPT_PATH}`

CS=15
FMD=5
MSDF=1500
ST=1.5

if [[ ! -d ${FOLDING_HOME} ]] ; then
	echo "Cannot locate FOLDING_HOME (should be ${FOLDING_HOME} ?)"
	exit
else
	export FOLDING_HOME=${FOLDING_HOME%/bin}
fi

#
# Depending on the compilation system, the libraries are either found in lib/
# or lib64/
#

if test -d ${FOLDING_HOME}/lib ; then
	export LD_LIBRARY_PATH+=:${FOLDING_HOME}/lib
elif test -d ${FOLDING_HOME}/lib64 ; then
	export LD_LIBRARY_PATH+=:${FOLDING_HOME}/lib64
fi

#
# Process optional parameters first
#

OUTPUTDIR=""

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
		shift 2
	elif  [[ "${1}" == "-source" ]] ; then
		echo Given source directory: `readlink -fn ${2}`
		SOURCE_DIRECTORY_SUFFIX="-source `readlink -fn ${2}`"
		shift 2
	elif [[ "${1}" == "-counter" ]] ; then
		REQUESTED_COUNTERS_TO_FOLD+=" -counter ${2}"
		shift 2
	elif [[ "${1}" == "-region" ]] ; then
		REQUESTED_REGIONS_TO_FOLD+=" -region ${2}"
		shift 2
	elif [[ "${1}" == "-extract-from" ]] ; then
		EXTRACT_ADDITIONAL_PARAMETERS+=" -extract-from ${2}"
		shift 2
	elif [[ "${1}" == "-extract-to" ]] ; then
		EXTRACT_ADDITIONAL_PARAMETERS+=" -extract-to ${2}"
		shift 2
	elif [[ "${1}" == "-pct" ]] ; then
		CS=${2}
		FMD=${3}
		shift 3
	elif [[ "${1}" == "-show-commands" ]] ; then
		SHOW_COMMANDS=yes
		shift
	elif [[ "${1}" == "-output" ]] ; then
		if [[ ! -d "${2}" ]] ; then
			mkdir -p ${2}
			if [[ ${?} -ne 0 ]] ; then
				echo "Could not create directory ${2}. Dying..."
				exit
			fi
		fi
		OUTPUTDIR=`readlink -fn ${2}`
		shift 2
	else
		break
	fi
done

#
# Check for mandatory parameters
#

if [[ $# -ne 2 ]] ; then
	echo "Usage: ${0} [-counter C] [-extract-from T] [-extract-to T] [-output <dir>] [-model M] [-source S] [-region R] [-pct CS FMD] Trace InstanceSeparator/SemanticSeparator"
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
	echo "            -counter C       : Applies the folding to the hardware counter C instead to all available counters"
	echo "            -extract-from T  : Applies the folding only after time T within the trace-file"
	echo "            -extract-to   T  : Applies the folding only up to time T within the trace-file"
	echo "            -source S        : Indicates where the source code of the application is located"
	echo "            -region R        : Requests to apply the folding to region R"
	echo "            -output <dir>    : Where to generate the results (if not given, they are generated in the tracefile <dir>)"
	echo "            -pct CS FMD         : Callstack processor parameters: number of consecutive samples CS, minimum duration (in percentage) FMD"
	echo "            Trace            : Paraver trace-file"
	echo "            InstanceSeparator: Label or value of the event type to separate instances within tracefile"
	echo "            SemanticSeparator: .csv file generated from Paraver to separate instances within tracefile"
	exit
fi

if [[ ! -f ${1} ]] ; then
	echo "Cannot access tracefile ${1}"
	exit
else
	PRVDIR=`dirname ${1}`
	if ! [[ "${PRVDIR}" == /* ]] ; then
		PRVDIR=${PWD}/${PRVDIR}
	fi
	if [[ "${OUTPUTDIR}" = "" ]] ; then
		OUTPUTDIR=${PRVDIR}
	fi
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
	EXTRA_INTERPOLATE_FLAGS="${REQUESTED_REGIONS_TO_FOLD}"
fi

EXTRA_INTERPOLATE_FLAGS+="${REQUESTED_COUNTERS_TO_FOLD}"

# BASENAME_PRV="${1%.*}"
BASENAME_PRV=${PRVBASE}

mkdir -p ${OUTPUTDIR}/${BASENAME_PRV}.folding || exit
cd ${OUTPUTDIR}/${BASENAME_PRV}.folding || exit

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
		echo ${FOLDING_HOME}/bin/extract ${EXTRACT_ADDITIONAL_PARAMETERS} -separator \"${2}\" \"${BASENAME_PRV}.codeblocks.fused.prv\"
	fi
	${FOLDING_HOME}/bin/extract ${EXTRACT_ADDITIONAL_PARAMETERS} -separator "${2}" "${BASENAME_PRV}.codeblocks.fused.prv" || exit
	EXTRA_INTERPOLATE_FLAGS+=" -feed-first-occurrence any"
else
	if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
		echo ${FOLDING_HOME}/bin/extract ${EXTRACT_ADDITIONAL_PARAMETERS} -semantic \"${BASENAME_CSV}.csv\" \"${BASENAME_PRV}.codeblocks.fused.prv\"
	fi
	${FOLDING_HOME}/bin/extract ${EXTRACT_ADDITIONAL_PARAMETERS} -semantic "${BASENAME_CSV}.csv" "${BASENAME_PRV}.codeblocks.fused.prv" || exit
fi

if [[ "${SHOW_COMMANDS}" = "yes" ]] ; then
	echo ${FOLDING_HOME}/bin/interpolate -max-samples-distance ${MSDF} -sigma-times ${ST} -callstack-processor pct ${CS} ${FMD} -use-median ${EXTRA_INTERPOLATE_FLAGS} ${MODELS_SUFFIX} ${SOURCE_DIRECTORY_SUFFIX} "${BASENAME_PRV}.codeblocks.fused.extract"
fi

# Hook for Paraver
echo Output directory: ${PWD}

${FOLDING_HOME}/bin/interpolate \
 -max-samples-distance-fast ${MSDF} \
 -sigma-times ${ST} \
 -use-median \
 -callstack-processor pct ${CS} ${FMD} \
 ${EXTRA_INTERPOLATE_FLAGS} \
 ${MODELS_SUFFIX} \
 ${SOURCE_DIRECTORY_SUFFIX} \
 "${BASENAME_PRV}.codeblocks.fused.extract"

cd ..

