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
#include "pcf-common.H"

#include "ParaverTrace.h"
#include "ParaverTraceThread.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceApplication.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <vector>

using namespace std;

string sourceDir = string ("/dev/null");

namespace libparaver {

class codeblock 
{
	private:
	unsigned beginline;
	unsigned endline;
	unsigned ID;
	bool inuse;

	public:
	codeblock (unsigned beginline, unsigned endline, unsigned id);
	codeblock& operator=( const codeblock& other );
	unsigned getBeginLine(void) const
	  { return beginline; }
	unsigned getEndLine(void) const
	  { return endline; }
	unsigned getID(void) const
	  { return ID; }
	bool inUse(void) const
	  { return inuse; }
	void setinUse(bool b)
	  { inuse = b; }
};

codeblock::codeblock (unsigned beginline, unsigned endline, unsigned id)
{
	this->beginline = beginline;
	this->endline = endline;
	this->ID = id;
	this->inuse = false;
}

codeblock& codeblock::operator=( const codeblock& other )
{
	beginline = other.beginline;
	endline = other.endline;
	ID = other.ID;
	inuse = other.inuse;
	return *this;
}

class Process : public ParaverTrace
{
	private:
	string sourceDir;
	ofstream traceout;
	map <string, vector< codeblock > > fileblocks;
	UIParaverTraceConfig *pcf;

	public:
	Process (string prvFile, string sourceDir, bool multievents, string tracename);
	~Process ();

	void prepare (void);
	void appendtoPCF (string file);

	bool is_f90 (string &f) const;
	bool is_cxx (string &f) const;
	bool is_c (string &f) const;

	void processState (struct state_t &s);
	void processMultiEvent (struct multievent_t &e);
	void processEvent (struct singleevent_t &e);
	void processCommunication (struct comm_t &c);
	void processCommunicator (string &c);
	void processComment (string &c);
};

bool Process::is_f90 (string &f) const
{
	if (f.length() >= 4)
		if (f.substr(f.length()-4) == ".f90" ||
		    f.substr(f.length()-4) == ".F90")
			return true;
	return false;}

bool Process::is_cxx (string &f) const
{
	if (f.length() >= 2)
		if (f.substr(f.length()-2) == ".C")
			return true;

	if (f.length() >= 4)
		if (f.substr(f.length()-4) == ".cpp" ||
		    f.substr(f.length()-4) == ".c++" ||
		    f.substr(f.length()-4) == ".cxx")
			return true;

	return false;
}

bool Process::is_c (string &f) const
{
	if (f.length() >= 2)
		if (f.substr(f.length()-2) == ".c")
			return true;
	return false;
}

Process::Process (string prvFile, string sourceDir, bool multievents, string tracename) : ParaverTrace (prvFile, multievents)
{
	traceout.open (tracename.c_str());
	if (!traceout.is_open())
	{
		cout << "Unable to create " << tracename << endl;
		exit (-1);
	}

	this->sourceDir = sourceDir;

	pcf = new UIParaverTraceConfig;
	pcf->parse (prvFile.substr (0, prvFile.length()-3) + string ("pcf"));

	try {
		vector<unsigned> SampleLocations = pcf->getEventValues (30000000);
	} catch ( ... ) {
		cerr << "Unable to get sample locations. Did you miss adding source code references when generating the tracefile (-e flag in mpi2prv)?" << endl;
		exit (-1);
	}
}

Process::~Process ()
{
	traceout.close ();

	delete pcf;
}

void Process::prepare (void)
{
	assert (pcf!=NULL);

	vector<unsigned> SampleLocations = pcf->getEventValues (30000100);
	set <string> testedFiles;
	unsigned total_id = SampleLocations.size(); 

	string folding_home = getenv ("FOLDING_HOME");
	setenv ("FOLDING_CLANG_BIN", CLANG_BINARY, 1);

	for (unsigned i = 0; i < SampleLocations.size(); i++)
	{
		string file;
		unsigned line;

		/* Skip End, Not Found & Unresolved */
		if (SampleLocations[i] <= 2)
			continue;

		pcfcommon::lookForCallerLineInfo (pcf, SampleLocations[i], file, line);
		if (testedFiles.count (file) == 0 && common::existsFile (sourceDir+"/"+file))
		{
			cout << "Calculating AST code blocks for file " << sourceDir+"/"+file << endl;
			stringstream ss;
			ss << getpid(); 

			string sys_command;
			if (is_f90(file))
				sys_command = folding_home+"/etc/basicblocks.F90.py "+sourceDir+"/"+file+"> /tmp/astblocks."+ss.str()+".out";
			else if (is_c(file) || is_cxx(file))
				sys_command = folding_home+"/etc/basicblocks.C.py "+sourceDir+"/"+file+"> /tmp/astblocks."+ss.str()+".out";

			int res = system (sys_command.c_str());
			if (res == 0 && common::existsFile (string("/tmp/astblocks.")+ss.str()+".out"))
			{
				vector< codeblock > vtmp;
				ifstream bb_file((string("/tmp/astblocks.")+ss.str()+".out").c_str());
				unsigned preline = 1, lastline;
				while (bb_file >> lastline)
				{
					codeblock c(preline, lastline, total_id++);
					vtmp.push_back (c);
					preline = lastline;
				}
				bb_file.close();
				unlink ((string("/tmp/astblocks.")+ss.str()+".out").c_str());
				fileblocks[file] = vtmp;
			}
			testedFiles.insert (file);
		}
		else if (testedFiles.count (file) == 0 && !common::existsFile (sourceDir+"/"+file))
		{
			cout << "Can't calculate AST code blocks for file " << sourceDir << "/" << file << endl;
			testedFiles.insert (file);
		}
	}

	unsetenv ("FOLDING_CLANG_BIN");
}

void Process::processComment (string &c)
{
	if (c.substr(0, strlen("Paraver")) != "Paraver")
		traceout << "# " << c << endl;
	else
		traceout << "#" << c << endl;
}

void Process::processCommunicator (string &c)
{
	traceout << "c" << c << endl;
}

void Process::processState (struct state_t &s)
{
	traceout << "1:"
	         << s.ObjectID.cpu << ":" << s.ObjectID.ptask << ":" << s.ObjectID.task << ":" << s.ObjectID.thread << ":"
	         << s.Begin_Timestamp << ":" << s.End_Timestamp << ":" << s.State << endl; 
} 

void Process::processMultiEvent (struct multievent_t &e)
{
	traceout << "2:" 
           << e.ObjectID.cpu << ":" << e.ObjectID.ptask << ":" << e.ObjectID.task << ":" << e.ObjectID.thread << ":"
           << e.Timestamp;

	for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
	{
		traceout << ":" << (*it).Type << ":" << (*it).Value;
		if ((*it).Type >= 30000100 && (*it).Type <= 30000199)
		{
			string file;
			unsigned line;

			pcfcommon::lookForCallerLineInfo (pcf, (*it).Value, file, line);

			if (common::DEBUG())
				cout << "Looking for CallerLine info for value " << (*it).Value << " file = "
				  << file << " at line " << line << " at timestamp " << e.Timestamp << endl;

			int newvalue;
			if (fileblocks.count(file) > 0)
			{
				bool within_astblock = false;
				vector< codeblock > v = fileblocks[file];
				for (unsigned u = 0; u < v.size(); u++)
				{
					codeblock c = v[u];
					if (line >= c.getBeginLine() && line <= c.getEndLine() )
					{
						newvalue = c.getID();
						c.setinUse (true);

						/* Store any change back into vector & map */
						v[u] = c;
						fileblocks[file] = v;

						within_astblock = true;
						break;
					}
				}
				if (!within_astblock)
					newvalue = 0;
			}
			else
				newvalue = (*it).Value;

			int delta = (*it).Type - 30000100;
			traceout << ":" << 30000200 + delta << ":" << newvalue;
		}
	}
	traceout << endl;
}

void Process::processEvent (struct singleevent_t &e)
{
	e = e;
}

void Process::processCommunication (struct comm_t &c)
{
	traceout << "3:" 
		<< c.Send_ObjectID.cpu << ":" << c.Send_ObjectID.ptask << ":" << c.Send_ObjectID.task << ":" << c.Send_ObjectID.thread << ":"
		<< c.Logical_Send << ":" << c.Physical_Send << ":"
		<< c.Recv_ObjectID.cpu << ":" << c.Recv_ObjectID.ptask << ":" << c.Recv_ObjectID.task << ":" << c.Recv_ObjectID.thread << ":"
		<< c.Logical_Recv << ":" << c.Physical_Recv << ":"
		<< c.Size << ":" << c.Tag << endl;
}

void Process::appendtoPCF (string file)
{
	ofstream PCFfile;
	PCFfile.open (file.c_str(), ios_base::out|ios_base::app);
	if (!PCFfile.is_open())
	{
		cerr << "Unable to append to: " << PCFfile << endl;
		exit (-1);
	}

	bool finish = false;
	unsigned ndepth = 0;
	for (unsigned u = 30000100; !finish && u < 30000199; u++)
	{
		try
		{
			vector<unsigned> SampleLocations = pcf->getEventValues (u);
			ndepth++;
		}
		catch (...)
		{
			finish = true;
		}
	}

	vector<unsigned> SampleLocations = pcf->getEventValues (30000100);
	PCFfile << endl <<  "EVENT_TYPE" << endl;
	for (unsigned u = 0; u < ndepth; u++)
	{
		string extra = "";
		if (u > 0)
		{
			stringstream ss;
			ss << u;
			extra = " (depth " + ss.str() + ")";
		}
		PCFfile << "0 " << 30000200 + u << " " << "Code block for sampled line" << extra << endl;
	}
	PCFfile << "VALUES" << endl;
	for (unsigned i = 0; i < SampleLocations.size(); i++)
		PCFfile << i << " " << pcf->getEventValue(30000100, i) << endl;

	map <string, vector< codeblock > > :: iterator it;
	for (it = fileblocks.begin(); it != fileblocks.end(); it++)
	{
		vector < codeblock > vblocks = (*it).second;
		for (unsigned u = 0; u < vblocks.size(); u++)
		{
			codeblock c = vblocks[u];
			if (c.inUse())
				PCFfile << c.getID() << " " << c.getBeginLine() << "-" << c.getEndLine() << " (" << (*it).first << ")" << endl;
		}
	}
	PCFfile << endl;
	PCFfile.close();
}


} /* namespace libparaver */

int ProcessParameters (int argc, char *argv[])
{
#define CHECK_ENOUGH_ARGS(N, argc, i) \
	(((argc) - 1 - (i)) > (N))

	if (argc < 2)
	{
		cerr << "Insufficient number of parameters" << endl
		     << "Available options are: " << endl
		     << "-source DIR [where DIR points to the application source code]" << endl
		     << endl;
		exit (-1);
	}

	for (int i = 1; i < argc-1; i++)
	{
		if (strcmp ("-source", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -source parameter" << endl;
				exit (-1);
			}
			i++;
			sourceDir = argv[i];
			continue;
		}
		else
			cout << "Misunderstood parameter: " << argv[i] << endl;
	}

	return argc-1;
#undef CHECK_ENOUGH_ARGS
}


using namespace::libparaver;
using namespace::std;

int main (int argc, char *argv[])
{
	string tracename;

	if (getenv ("FOLDING_HOME") == NULL)
	{
		cerr << "You must define FOLDING_HOME to execute this application" << endl;
		return -1;
	}

	int res = ProcessParameters (argc, argv);
	tracename = string(argv[res]);
	if (!common::existsFile(tracename))
	{
		cerr << "The tracefile " << tracename << " does not exist!" << endl;
		return -2;
	}

	string bfileprefix = common::basename (tracename.substr (0, tracename.rfind(".prv")));

	Process *p = new Process (tracename, sourceDir, true, (bfileprefix + string(".codeblocks.prv")).c_str() );

	vector<ParaverTraceApplication *> va = p->get_applications();
	if (va.size() != 1)
	{
		cerr << "ERROR Cannot parse traces with more than one application" << endl;
		return -1;
	}

	if (sourceDir != "/dev/null")
		p->prepare();

	p->parseBody();

	/* Copy .pcf and .row files */
	ifstream ifs_pcf ((tracename.substr (0, tracename.rfind(".prv"))+string(".pcf")).c_str());
	if (ifs_pcf.is_open())
	{
		ofstream ofs_pcf ( (bfileprefix + string(".codeblocks.pcf")).c_str());
		ofs_pcf << ifs_pcf.rdbuf();
		ifs_pcf.close();
		ofs_pcf.close();
	}
	p->appendtoPCF (bfileprefix + string(".codeblocks.pcf"));

	ifstream ifs_row ((tracename.substr (0, tracename.rfind(".prv"))+string(".row")).c_str());
	if (ifs_row.is_open())
	{
		ofstream ofs_row ( (bfileprefix + string(".codeblocks.row")).c_str());
		ofs_row << ifs_row.rdbuf();
		ifs_row.close();
		ofs_row.close();
	}

	return 0;
}

