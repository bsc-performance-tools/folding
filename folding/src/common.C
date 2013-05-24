/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                   Folding                                 *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\
 | @file: $HeadURL$
 | 
 | @last_commit: $Date$
 | @version:     $Revision$
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id$";

#include "common.H"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define PAPI_MIN_COUNTER   42000000
#define PAPI_MAX_COUNTER   42999999

string common::convertDouble (double d, int i)
{
	stringstream s;//create a stringstream
	s << fixed << setprecision(i) << d;//add number to the stream with selected precision
	return s.str();//return a string with the contents of the stream
}

string common::convertInt (int i)
{
	stringstream s;//create a stringstream
	s << i;//add number to the stream
	return s.str();//return a string with the contents of the stream
}

string common::removeSpaces (string &in)
{
	string tmp = in;
	for (string::iterator it = tmp.begin(); it != tmp.end(); it++)
		if (*it == ' ')
			*it = '_';
	return tmp;
}

bool common::existsFile (string file)
{
	struct stat buffer;
	stat(file.c_str(), &buffer);
	return (buffer.st_mode & S_IFMT) == S_IFREG;
}

bool common::existsDir (string dir)
{
	struct stat buffer;
	stat(dir.c_str(), &buffer);
	return (buffer.st_mode & S_IFMT) == S_IFDIR;
}

unsigned common::lookForCounter (string &name, UIParaverTraceConfig *pcf)
{
	/* Look for every counter in the vector its code within the PCF file */
	for (unsigned j = PAPI_MIN_COUNTER; j <= PAPI_MAX_COUNTER; j++)
	{
		try
		{
			string s = pcf->getEventType (j);
			if (s.length() > 0)
			{
				string tmp = s.substr (s.find ('(')+1, s.find (')', s.find ('(')+1) - (s.find ('(') + 1));
				if (tmp == name)
					return j;
			}
		}
		catch (...)
		{ }
	}
	return 0;
}

void common::lookForCallerLineInfo (UIParaverTraceConfig *pcf, unsigned id, string &file, int &line)
{
	string cl = pcf->getEventValue (30000100, id);
	line = atoi ((cl.substr (0, cl.find (" "))).c_str());
	int pos_open = cl.find ("(");
	int pos_close = cl.find (")");
	file = cl.substr (pos_open+1, pos_close-pos_open-1);
}

void common::CleanMetricsDirectory_r (char *dir)
{
	struct dirent *de = NULL;
	DIR *d = NULL;
	char current[PATH_MAX];
	getcwd (current, PATH_MAX);
	struct stat sb;
	
	d = opendir(dir);
	if (d == NULL)
	{
		cerr << "Could not open directory " << dir << endl;
		return;
	}
	chdir (dir);

	while (de = readdir(d))
	{
		int res = stat (de->d_name, &sb);
		if (res > 0 && (sb.st_mode & S_IFMT) == S_IFREG)
		{
			if (strlen(de->d_name) > strlen (".metrics"))
				if (strcmp (&de->d_name[strlen(de->d_name)-strlen (".metrics")], ".metrics") == 0)
					unlink (de->d_name);
		}
	}

	rewinddir (d);

	while (de = readdir(d))
	{
		int res = stat (de->d_name, &sb);
		if (res > 0 && strcmp (de->d_name, ".") != 0 && strcmp (de->d_name, "..") != 0)
		{
			if ((sb.st_mode & S_IFMT) == S_IFDIR)
			{
				chdir (de->d_name);
				common::CleanMetricsDirectory_r (de->d_name);
				chdir (current);
			}
		}
	}

  closedir(d);
}

void common::CleanMetricsDirectory (string &directory)
{
	char current[PATH_MAX];
	getcwd (current, PATH_MAX);

	common::CleanMetricsDirectory_r ((char*)directory.c_str());
	chdir (current);
}

bool common::isMIPS (string s)
{
	return s == "PAPI_TOT_INS" || s == "PM_INST_CMPL" || s == "INSTRUCTION_RETIRED" || s == "INSTRUCTIONS_RETIRED";
}
