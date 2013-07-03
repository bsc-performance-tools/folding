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
#include <algorithm>
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
#if defined(HAVE_ACCESS)
	return access (file.c_str(), F_OK) == 0;
#elif defined(HAVE_STAT64)
	struct stat64 sb;
	stat64 (file.c_str(), &sb);
	return (sb.st_mode & S_IFMT) == S_IFREG;
#elif defined(HAVE_STAT)
	struct stat sb;
	stat (file.c_str(), &sb);
	return (sb.st_mode & S_IFMT) == S_IFREG;
#else
	int fd = open (file.c_str(), O_RDONLY);
	if (fd >= 0)
	{
		close (fd);
		return TRUE;
	}
	else
		return FALSE;
#endif
}

bool common::existsDir (string dir)
{
	struct stat buffer;
	stat(dir.c_str(), &buffer);
	return (buffer.st_mode & S_IFMT) == S_IFDIR;
}

unsigned common::lookForCounter (string &name, UIParaverTraceConfig *pcf)
{
	vector<unsigned> vtypes = pcf->getEventTypes();

	for (unsigned u = 0; u < vtypes.size(); u++)
	{
		if (vtypes[u] >= PAPI_MIN_COUNTER && vtypes[u] <= PAPI_MAX_COUNTER)
		{
			string s = pcf->getEventType (vtypes[u]);
			if (s.length() > 0)
			{
				string tmp = s.substr (s.find ('(')+1, s.find (')', s.find ('(')+1) - (s.find ('(') + 1));
				if (tmp == name)
					return vtypes[u];
			}
		}
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

bool common::DEBUG (void)
{
	static bool unset = true;
	static bool debug = false;

	if (unset)
	{
		debug = (getenv("DEBUG") != NULL);
		unset = false;
	}

	return debug;
}

bool common::decomposePtaskTaskThread (string &s, unsigned &ptask,
	unsigned &task, unsigned &thread)
{
	size_t n = count (s.begin(), s.end(), '.');
	if (n != 2)
		return false;

	string ptask_tmp = s.substr (0, s.find ("."));
	string task_tmp = s.substr (s.find(".")+1, s.rfind(".")-s.find(".")-1);
	string thread_tmp = s.substr (1+s.rfind("."));

	if ((ptask = atoi(ptask_tmp.c_str())) == 0)
		return false;
	if ((task = atoi(task_tmp.c_str())) == 0)
		return false;
	if ((thread = atoi(thread_tmp.c_str())) == 0)
		return false;

	return true;
}

bool common::decomposePtaskTaskThreadWithAny (string &s, unsigned &ptask,
	bool &anyptask, unsigned &task, bool &anytask, unsigned &thread,
	bool &anythread)
{
	size_t n = count (s.begin(), s.end(), '.');
	if (n != 2)
		return false;

	anyptask = anytask = anythread = false;

	string ptask_tmp = s.substr (0, s.find ("."));
	string task_tmp = s.substr (s.find(".")+1, s.rfind(".")-s.find(".")-1);
	string thread_tmp = s.substr (1+s.rfind("."));

	if (ptask_tmp == "*")
		anyptask = true;
	else
		if ((ptask = atoi(ptask_tmp.c_str())) == 0)
			return false;

	if (task_tmp == "*")
		anytask = true;
	else
		if ((task = atoi(task_tmp.c_str())) == 0)
			return false;

	if (thread_tmp == "*")
		anythread = true;
	else
		if ((thread = atoi(thread_tmp.c_str())) == 0)
			return false;

	return true;
}


