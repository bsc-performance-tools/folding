#!/usr/bin/python

import sys;
import re;

p_enddo = re.compile ("[^!]*end(\s)*do.*", re.IGNORECASE); # Match enddo - end do 
p_do_index = re.compile ("[^!]*do(\s)+.+(\s)*=.*,.*", re.IGNORECASE); # Match do X = X1, X2
p_do_while = re.compile ("[^!]*do(\s)+while[(].*[)].*", re.IGNORECASE); # Match do while ( .. )
p_do = re.compile ("([^!]*(\s)+do(\s)*:?|^do)", re.IGNORECASE); # Match do / do ; (first OR is to differentiate between DO in column)
p_endsubroutine = re.compile ("[^!]*end(\s)*subroutine.*",re.IGNORECASE); # Match end subroutine
p_subroutine = re.compile ("[^!]*subroutine.*", re.IGNORECASE); # Match enddo - end do 

currentline = 0;
lines = [];
sourcefile = sys.argv[1];
content = open (sourcefile, "r");

for line in content:

	currentline = currentline + 1;

	if p_enddo.match(line):
		lines.append(currentline)
	elif p_do_index.match(line):
		lines.append(currentline)
	elif p_do_while.match(line):
		lines.append(currentline)
	elif p_do.match(line):
		lines.append(currentline)
	elif p_endsubroutine.match(line):
		lines.append(currentline)
	elif p_subroutine.match(line):
		lines.append(currentline)

if currentline > 0:
	lines.append(currentline)

if len(lines) > 0:
	s = sorted(lines)
	for r in s:
		print r;
	

