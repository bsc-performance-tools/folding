#!/bin/bash

# Looks like Paraver auto-distributed shared libraries conflict with 
# shared libraries from gnuplot (called in the *folding-paraver-alienapp.bash
# script)
# As a workaround, we unset the LD_LIBRARY_PATH set by Paraver

unset LD_LIBRARY_PATH

trace_path=`dirname ${PARAVER_ALIEN_TRACE_FULL_PATH}`
trace_file_wo_prefix=`basename ${PARAVER_ALIEN_TRACE_FULL_PATH} .prv`
launcher=folding-paraver-alienapp-launcher.bash

if  [[ -x ${trace_path}/folding/${trace_file_wo_prefix}/${launcher} ]] ; then
# Has Paraver generated the folding results? They are located within ../folding/trace/..
	cd ${trace_path}/folding/${trace_file_wo_prefix}
elif [[ -x ${trace_path}/${trace_file_wo_prefix}/${launcher} ]] ; then
# Has the user manually run the folding? They must be located within ../trace/..
	cd ${trace_path}/${trace_file_wo_prefix}
else
# The folding results have not been found
	exit
fi

./${launcher} ${@:1}

