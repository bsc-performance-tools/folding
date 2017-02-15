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

#include "common.H"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
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

const string common::DefaultTimeUnit = "Time";

string common::convertDouble (double d, int i)
{
	stringstream s;//create a stringstream
	s << fixed << setprecision(i) << d;//add number to the stream with selected precision
	return s.str();//return a string with the contents of the stream
}

string common::convertInt (unsigned u)
{
	stringstream s;//create a stringstream
	s << u;//add number to the stream
	return s.str();//return a string with the contents of the stream
}

string common::convertInt (int i)
{
	stringstream s;//create a stringstream
	s << i;//add number to the stream
	return s.str();//return a string with the contents of the stream
}

string common::convertInt (size_t st)
{
	stringstream s;//create a stringstream
	s << st;//add number to the stream
	return s.str();//return a string with the contents of the stream
}

string common::removeSpaces (const string &in)
{
	string tmp = in;
	for (auto & c : tmp)
		if (c == ' ')
			c = '_';
	return tmp;
}

string common::removeUnwantedChars (const string &in)
{
	string tmp = in;
	for (auto & c : tmp)
		if (c == ' ' || c == '[' || c == ']' || c == '(' || c == ')'
		 || c == '"' || c == '*' || c == '/' || c == ':' || c == '<' || c == '>' || c == '?' || c == '?' || c == '\\' || c == '|' // NTFS
		)
			c = '_';
	return tmp;
}

bool common::existsFile (const string & file)
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

bool common::existsDir (const string & dir)
{
	struct stat buffer;
	stat(dir.c_str(), &buffer);
	return (buffer.st_mode & S_IFMT) == S_IFDIR;
}

void common::CleanMetricsDirectory_r (const char *dir)
{
	struct dirent *de = NULL;
	DIR *d = NULL;
	char current[PATH_MAX];
	char *c = getcwd (current, PATH_MAX);
	struct stat sb;
	
	d = opendir(dir);
	if (d == NULL)
	{
		cerr << "Could not open directory " << dir << endl;
		return;
	}
	if (chdir (dir) != 0)
	{
		cerr << "Could not change directory to " << dir << endl;
		perror ("");
		return;
	}

	while ((de = readdir(d)))
	{
		int res = stat (de->d_name, &sb);
		if (res == 0 && (sb.st_mode & S_IFMT) == S_IFREG)
			if (strlen(de->d_name) > strlen (".metrics"))
				if (strcmp (&de->d_name[strlen(de->d_name)-strlen (".metrics")], ".metrics") == 0)
					unlink (de->d_name);
	}

	rewinddir (d);

	while ((de = readdir(d)))
	{
		int res = stat (de->d_name, &sb);
		if (res > 0 && strcmp (de->d_name, ".") != 0 && strcmp (de->d_name, "..") != 0)
		{
			if ((sb.st_mode & S_IFMT) == S_IFDIR)
			{
				if (chdir (de->d_name) != 0)
				{
					cerr << "Could not change directory to " << de->d_name << endl;
					perror ("");
					return;
				}
				common::CleanMetricsDirectory_r (de->d_name);
				if (chdir (c) != 0)
				{
					cerr << "Could not change directory to " << c << endl;
					perror ("");
					return;
				}
			}
		}
	}

	closedir(d);
}

void common::CleanMetricsDirectory (const string & directory)
{
	char current[PATH_MAX];
	char *res = getcwd (current, PATH_MAX);

	common::CleanMetricsDirectory_r ((char*)directory.c_str());
	if (chdir (res) != 0)
	{
		cerr << "Could not change directory to " << res << endl;
		perror ("");
	}
}

bool common::isMIPS (const string & s)
{
	return s == "PAPI_TOT_INS" ||
	  s == "PM_INST_CMPL" || /* PPC */
	  s == "INSTRUCTION_RETIRED" || /* x86 */
	  s == "INSTRUCTIONS_RETIRED" || /* x86 */ 
	  s == "PEVT_INST_ALL"; /* PPC BGQ */
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

bool common::decomposePtaskTaskThread (const string &s, unsigned &ptask,
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

bool common::decomposePtaskTaskThreadWithAny (const string & s, unsigned &ptask,
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

string common::basename (const string & s)
{
	string r;

	if (s.length() > 0)
	{
		if (s.rfind ('/') != string::npos)
			r = s.substr (s.rfind ('/')+1);
		else
			r = s;
	}

	return r;
}

unsigned common::getNumCores (bool & found)
{
	unsigned ncores = 0;
	found = false;

	ifstream procinfo ("/proc/cpuinfo");
	if (procinfo.is_open())
	{
		string line;
		while (getline(procinfo, line) && !found)
		{
			if (line.find ("cpu cores") == 0) // at the begin of the line
				if (line.find (":") != string::npos)
				{
					string tmp = line.substr (line.find (":")+1);
					ncores = atoi (tmp.c_str());
					found = true;
				}
		}
		procinfo.close();
	}
	return ncores;
}

unsigned common::getNumProcessors (bool & found)
{
	found = false;
	unsigned nprocessors = 0;

	ifstream procinfo ("/proc/cpuinfo");
	if (procinfo.is_open())
	{
		string line;
		while (getline(procinfo, line))
			if (line.find ("processor") == 0) // at the begin of the line
				nprocessors++;
		procinfo.close();
	}
	return nprocessors;
}

unsigned common::numDigits (unsigned long long v, unsigned base)
{
	if (v > 0)
	{
		unsigned res = 0;
		do
		{
			v = v / base;
			res++;
		} while (v > 0);

		return res;
	}
	else
		return 1;
}

unsigned common::numDecimalDigits (unsigned long long v)
{
	return numDigits (v, 10);
}

unsigned common::numHexadecimalDigits (unsigned long long v)
{
	return numDigits (v, 16);
}

bool common::addressInStack (unsigned long long address)
{
	return address & (0xfffULL << 32);
}

