#!/usr/bin/python

import sys;
import re;

p_enddo = re.compile ("[^!]*end(\s)*do.*", re.IGNORECASE); # Match enddo - end do 
p_do = re.compile ("[^!]*do(\s)+.+(\s)*=.*,.*", re.IGNORECASE); # Match do X = X1, X2
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
	elif p_do.match(line):
		lines.append(currentline)
	elif p_endsubroutine.match(line):
		lines.append(currentline)
	elif p_subroutine.match(line):
		lines.append(currentline)

if maxline > 0:
	lines.append(maxline)

if len(lines) > 0:
	s = sorted(lines)
	for r in s:
		print r;
	

