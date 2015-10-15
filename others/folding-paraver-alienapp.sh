#!/bin/bash

where=`find $PWD -name "*.folding-paraver-alienapp.bash"`

if test "${where}" != "" ; then
	cd `dirname ${where}`
	e=`basename ${where}`
	./${e} ${@:1}
fi
