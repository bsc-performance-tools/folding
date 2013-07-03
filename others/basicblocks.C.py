#!/usr/bin/python

import exceptions;
import sys;
import re;
import os;

def tonum (s):
	try:
		return int(s)
	except exceptions.ValueError:
		return int(0)

def decode1(line, what):
	line_start_pos = line.find(what) + len(what) + 1;
	line_end_pos = line.find(":", line_start_pos);
	return [tonum(line[line_start_pos:line_end_pos]), tonum(line[line_start_pos:line_end_pos])];

def decode2(line, what1, what2):
	line1_start_pos = line.find(what1) + len(what1) + 1
	line1_end_pos = line.find(":", line1_start_pos);
	line2_start_pos = line.find(what2, line1_end_pos) + len(what2) + 1;
	line2_end_pos = line.find(":", line2_start_pos);
	return [tonum(line[line1_start_pos:line1_end_pos]), tonum(line[line2_start_pos:line2_end_pos])];

# START POINT

envvars = os.environ
if not ('FOLDING_CLANG_BIN' in envvars):
	print "Cannot obtain clang binary through FOLDING_CLANG_BIN env variable";
	os._exit(1)

CLANGBIN = envvars['FOLDING_CLANG_BIN'];

#pall = re.compile (".*\((.*ForStmt.*|.*WhileStmt.*|.*DoStmt.*|.*SwitchStmt.*|.*CaseStmt.*|.*IfStmt.*|.*CompoundStmt.*|.*CallExpr.*|.*LabelStmt.*|.*GotoStmt.*)")
pall = re.compile (".*\((.*ForStmt.*|.*WhileStmt.*|.*DoStmt.*|.*SwitchStmt.*|.*CaseStmt.*|.*IfStmt.*|.*CompoundStmt.*|.*LabelStmt.*|.*GotoStmt.*)");

maxline = 0;
lines = [];
sourcefile = sys.argv[1];
stdinput, ASTcontent, stderr = os.popen3 (CLANGBIN+" -cc1 -ast-dump "+sourcefile);

for line in ASTcontent:
	if pall.match(line):
		beginpos = line.find ("line:");
		endpos = line.find (">");
		if beginpos > 0 and endpos > 0:
			if line.find(sourcefile) > 0: # The statement contains the sourcefile (this is a new routine!)
				lines = lines + decode2 (line, sourcefile, "line")
			else:
				subline = line[beginpos:endpos]
				countline = line.count ("line:")
				if countline == 1: # The statement is in a single line 
					lines = lines + decode1 (line, "line")
				elif countline == 2: # The statement is within two lines
					lines = lines + decode2 (line, "line", "line")

if maxline > 0:
	lines.append(maxline)

if len(lines) > 0:
	s = sorted(lines)
	for r in s:
		print r;
	

