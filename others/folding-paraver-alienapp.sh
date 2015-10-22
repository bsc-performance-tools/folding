#!/bin/bash

# Looks like Paraver auto-distributed shared libraries conflict with 
# shared libraries from gnuplot (called in the *folding-paraver-alienapp.bash
# script)
# As a workaround, we unset the LD_LIBRARY_PATH set by Paraver

unset LD_LIBRARY_PATH

trace_path=`dirname ${PARAVER_ALIEN_TRACE_FULL_PATH}`
where=`find ${PWD}/ -name "*.folding-paraver-alienapp.bash"`

if test "${where}" != "" ; then
	cd `dirname ${where}`
	e=`basename ${where}`
	./${e} ${@:1}
fi
