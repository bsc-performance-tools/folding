#!/usr/bin/env python

import exceptions;
import sys;
import re;
import os;

def tofloat (s):
	try:
		return float(s)
	except exceptions.ValueError:
		return float(0)

contents = {}
durations = {}
maxduration = 0.0

pX_LIMIT = re.compile ("^X_LIMIT=.* # Do not touch this");
pFACTOR = re.compile ("^FACTOR=.* # Do not touch this");

if len (sys.argv) < 2: 
	print 'Error! Command ' + sys.argv[0] + ' requires multiple gnuplots as a paremeter'
	sys.exit (-1)

elif len (sys.argv) >= 2:

	# Read contents of the given files
	for f in sys.argv:
		h = open (f, 'r')
		try:
			line = h.readlines()
			contents[f] = line
		finally:
			h.close()

	# Look for durations in all given plots
	for f in sys.argv[1:]:
		lines = contents[f]
		found = False
		for l in lines:
			if pX_LIMIT.match(l):
				found = True
				beginpos = len ("X_LIMIT=")
				endpos = l.find (" # Do not touch this")
				durations[f] = tofloat (l[beginpos:endpos])
				if durations[f] > maxduration:
					maxduration = durations[f]
		if not found:
			print 'Error! File '+f+' does not contain X_LIMIT';
			dsys.exit (-1);

	# Generate the output combined file

	# Emit header first
	print "# set term postscript eps solid color size 3,", str (1.5*(len(sys.argv)-1))
	print "# set term pdfcairo solid color lw 2 font \",9\" size 3,", str (1.5*(len(sys.argv)-1))
	print "# set term png size 800,",str(300*(len(sys.argv)-1))
	print "# set term x11 size 800,",str(300*(len(sys.argv)-1))
	print "# set term wxt size 800,",str(300*(len(sys.argv)-1))
	print ""
	print "set multiplot layout ",str(len(sys.argv)-1), ",1"
	print ""

	# Emit bodies after
	for f in sys.argv[1:]:
		print ""
		print "# Contents of file "+f
		print ""
		lines = contents[f]
		for l in lines:
			if pFACTOR.match(l):
				print "FACTOR="+str(durations[f])+"/"+str(maxduration)+ " # Do not touch this"
			else:
				print l.strip()

	# Finish multiplot 
	print "unset multiplot;"

