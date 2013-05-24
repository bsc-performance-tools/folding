#!/bin/bash

if test -z ${FOLDING_HOME} ; then
	echo "Error! Environment variable FOLDING_HOME is not set!"
	exit -1
fi

if test -z ${PARAVER_HOME} ; then
	echo "Error! Environment variable PARAVER_HOME is not set!"
	exit -1
fi

if test $# -ne 7 ; then
	echo "Error! Invalid number of parameters"
	echo "Pass: tracefile, begin time, end time, counterid, definition of counterid, separator and definition of separator"
	exit -2
fi

TRACEFILE=${1}
BEGIN_TIME=${2}
END_TIME=${3}
COUNTERID=${4}
DEF_COUNTERID=${5}
SEPARATOR=${6}
DEF_SEPARATOR=${7}

#WXPARAVER_BIN_PROCESS=`ps -u ${USER} | grep wxparaver.bin | grep -v grep | cut -f 2 -d" "`
#if test "${WXPARAVER_BIN_PROCESS}" = "" ; then
#	${PARAVER_HOME}/bin/wxparaver &
#	sleep 3
#	WXPARAVER_BIN_PROCESS=`ps -u ${USER} | grep wxparaver.bin | grep -v grep | cut -f 2 -d" "`
#fi
#echo "WXPARAVER is ${WXPARAVER_BIN_PROCESS}"

TMPCFGFILE=`mktemp --suffix=.cfg`

#echo ${TMPCFGFILE} > ${HOME}/paraload.sig
#echo ${BEGIN_TIME}:${END_TIME} >> ${HOME}/paraload.sig
#echo ${TRACEFILE} >> ${HOME}/paraload.sig

sed s/@WBT@/${BEGIN_TIME}/ < ${FOLDING_HOME}/etc/cube-call-paraver-base.cfg | \
 sed s/@WET@/${END_TIME}/ | \
 sed s/@COUNTER@/${COUNTERID}/ | \
 sed s/@COUNTER_DEFINITION@/${DEF_COUNTERID}/ | \
 sed s/@SEPARATOR@/${SEPARATOR}/ | \
 sed s/@SEPARATOR_DEFINITION@/${DEF_SEPARATOR}/ > ${TMPCFGFILE}

${PARAVER_HOME}/bin/wxparaver ${TRACEFILE} ${TMPCFGFILE} &

#kill -USR1 ${WXPARAVER_BIN_PROCESS}

