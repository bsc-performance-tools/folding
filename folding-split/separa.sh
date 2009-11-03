#!/bin/bash

if test $# -ne 3 ; then
	echo "Usage ${0} datafile counter pcffile"
	exit 0
fi

TMPFILE1=`mktemp -p .`
TMPFILE2=`mktemp -p .`
TMPFILE3=`mktemp -p .`
GNUPLOTfile=all-phases.gnuplot

TITLE=`grep $2 $3 | cut -f3-10 --delimiter=\ `

grep $2 $1 | grep INPOINTS2 > ${TMPFILE1}
grep $2 $1 | grep KRI-AC1   > ${TMPFILE2}
grep $2 $1 | grep KRI-AC3   > ${TMPFILE3}

rm -f ${GNUPLOTfile}
echo "set title '${TITLE}'" >> ${GNUPLOTfile}
echo "set xrange [0:1]" >> ${GNUPLOTfile}
echo "set yrange [0:1]" >> ${GNUPLOTfile}
echo "set x2range [0:1]" >> ${GNUPLOTfile}
echo "set y2range [0:2]" >> ${GNUPLOTfile}
echo "set ytics nomirror" >> ${GNUPLOTfile}
echo "set y2tics" >> ${GNUPLOTfile}
echo "set xtics nomirror" >> ${GNUPLOTfile}
echo "set x2tics" >> ${GNUPLOTfile}
echo "set ylabel 'Performance counter value'" >> ${GNUPLOTfile}
echo "set y2label 'Performance counter slope'" >> ${GNUPLOTfile}
echo "set key left top" >> ${GNUPLOTfile}

if test -s ${TMPFILE1} -a -s ${TMPFILE2} ; then
	for (( i = 0; i < 200; i++))
	do
		grep " phase $i " ${TMPFILE1} > inpoints-$i
	  if ! test -s inpoints-$i ; then
			rm inpoints-$i
		else
			grep " phase $i " ${TMPFILE2} > kri-ac1-$i
			grep " phase $i " ${TMPFILE3} > kri-ac3-$i

			if test -s kri-ac1-$i ; then
				echo "plot 'inpoints-$i' using 6:8 title 'samples' with points pointsize 2, 'kri-ac1-$i' using 5:7 title 'curve fitting' with linespoints linewidth 2, 'kri-ac3-$i' using 5:7 title 'curve fitting slope' axes x2y2 with linespoints;" >> ${GNUPLOTfile}
				echo "pause -1;" >> ${GNUPLOTfile}
			fi
		fi
	done
fi

rm -f ${TMPFILE1} ${TMPFILE2} ${TMPFILE3}

