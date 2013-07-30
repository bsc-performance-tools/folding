#!/bin/bash

FOLDING_HOME=

if test $# -ne 3 ; then
	echo "Invalid number of arguments. You must provide:"
	echo "  1) type to fold (e.g. 90000001 for clusters, 60000019 for user functions)"
	echo "  2) trace prefix (trace without .prv extension)"
	echo "  3) which object to fold (e.g. 1.2.4 to reference application 1, process 2, and thread 4)"
	echo
	exit
fi

WHAT=$1
TRACE_PREFIX=$2
OBJECT=$3
DIRNAME=`basename $2`

mkdir -p $DIRNAME
cp $TRACE_PREFIX.prv $TRACE_PREFIX.pcf $TRACE_PREFIX.row $DIRNAME
cd $DIRNAME

$FOLDING_HOME/bin/codeblocks $TRACE_PREFIX.prv
$FOLDING_HOME/bin/fuse $TRACE_PREFIX.codeblocks.prv
TRACE_PREFIX=$TRACE_PREFIX.codeblocks.fused
$FOLDING_HOME/bin/extract -separator $WHAT $TRACE_PREFIX.prv
$FOLDING_HOME/bin/interpolate -sigma-times 1.5 -max-samples-distance 2000 $TRACE_PREFIX.extract

read -p "Press a key to plot $TRACE_PREFIX.slopes.*.gnuplot"
find . -name $TRACE_PREFIX.\*slopes.gnuplot -exec gnuplot -persist {} \;

cd ..
