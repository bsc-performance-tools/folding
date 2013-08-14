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

#include "ParaverTrace.h"
#include "ParaverTraceThread.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceApplication.h"
#include "UIParaverTraceConfig.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <list>
#include <set>
#include <exception>

#include "interpolate.H"

unsigned long long RegionSeparator = 0;
string RegionSeparatorName;

using namespace std;

double NormalizeValue (double value, double min, double max)
{ 
  return (value-min) / (max-min);
} 
    
double DenormalizeValue (double normalized, double min, double max)
{     
  return (normalized*(max-min))+min;
}     

namespace libparaver {

class Sample
{
	public:
	map<unsigned, unsigned long long> CounterValues; /* Map to ID of counter to its value */
	map<unsigned, unsigned long long> Caller;        /* Map depth of caller */
	map<unsigned, unsigned long long> CallerLine;    /* Map depth of caller line */
	map<unsigned, unsigned long long> CallerLineAST; /* Map depth of caller line AST */
	unsigned long long Timestamp;
};

class ThreadInformation
{
	private:
	bool seen;

	public:
	vector<Sample> Samples;

	unsigned long long CurrentRegion;
	unsigned long long StartRegion;

	bool getSeen (void) const
	  { return seen; }
	void setSeen (bool b)
	  { seen = b; }

	ThreadInformation ();
	~ThreadInformation ();
};

ThreadInformation::ThreadInformation ()
{	
	seen = false;
	CurrentRegion = 0;
}

ThreadInformation::~ThreadInformation ()
{
}

class TaskInformation
{
	private:
	int numThreads;
	ThreadInformation *ThreadsInfo;

	public:
	int getNumThreads (void)
	{ return numThreads; };

	ThreadInformation* getThreadsInformation (void)
	{ return ThreadsInfo; };

	void AllocateThreads (int numThreads);
	~TaskInformation();
};

TaskInformation::~TaskInformation()
{
	delete [] ThreadsInfo;
}

void TaskInformation::AllocateThreads (int numThreads)
{
	this->numThreads = numThreads;
	ThreadsInfo = new ThreadInformation[this->numThreads];
}

class PTaskInformation
{
	private:
	int numTasks;
	TaskInformation *TasksInfo;

	public:
	int getNumTasks (void)
	{ return numTasks; };

	TaskInformation* getTasksInformation (void)
	{ return TasksInfo; };

	void AllocateTasks (int numTakss);
	~PTaskInformation();
};

PTaskInformation::~PTaskInformation()
{
	delete [] TasksInfo;
}

void PTaskInformation::AllocateTasks (int numTasks)
{
	this->numTasks = numTasks;
	TasksInfo = new TaskInformation[this->numTasks];
}


class InformationHolder
{
	private:
	int numPTasks;
	PTaskInformation *PTasksInfo;
	set<string> seenCounters;
	set<string> seenRegions;
	set<unsigned> missingRegions; /* Those regions that do not have an entry as a value in the PCF*/
	
	public:
	int getNumPTasks (void) const
	{ return numPTasks; };

	PTaskInformation* getPTasksInformation (void)
	{ return PTasksInfo; };

	void AllocatePTasks (int numPTasks);
	~InformationHolder();

	void addCounter (string c)
	  { seenCounters.insert (c); }
	void addRegion (string r)
	  { seenRegions.insert (r); }
	void addMissingRegion (unsigned r)
	  { missingRegions.insert (r); }
	set<string> getCounters (void)
	  { return seenCounters; }
	set<string> getRegions (void)
	  { return seenRegions; }
	set<unsigned> getMissingRegions (void)
	  { return missingRegions; }

	ofstream outputfile;
};

InformationHolder::~InformationHolder()
{
	delete [] PTasksInfo;
}

void InformationHolder::AllocatePTasks (int numPTasks)
{
	this->numPTasks = numPTasks;
	PTasksInfo = new PTaskInformation[this->numPTasks];
}

class Process : public ParaverTrace
{
	private:
	string pcffile;
	unsigned nCounterChanges;
	list<unsigned> CallerCut;
	string *CounterIDNames;
	unsigned long long *CounterIDs;
	bool *CounterUsed;
	bool *HackCounter;
	unsigned LookupCounter (unsigned long long Counter, bool &found);
	UIParaverTraceConfig *pcf;
	InformationHolder IH;
	unsigned numCounterIDs;

	bool checkSamples (vector<Sample> &Samples);
	void dumpSamples (unsigned ptask, unsigned task, unsigned thread,
	  unsigned long long region, unsigned long long start,
	  unsigned long long duration, vector<Sample> &Samples);
	bool equalTypes (map<unsigned, unsigned long long> &m1, map<unsigned, unsigned long long> &m2);
	void processCaller (struct event_t &evt, unsigned base, Sample &s);
	void processCounter (struct event_t &rvt, Sample &s);

	void dumpSeenObjects (string fnameprefix);
	void dumpSeenCounters (string fnameprefix);
	void dumpSeenRegions (string fnameprefix);

	public:
	Process (string prvFile, bool multievents);

	string getType (unsigned type, bool &found);
	string getTypeValue (unsigned type, unsigned value, bool &found);
	void allocateBuffers (void);
	void closeFile (void);
	void processState (struct state_t &s);
	void processMultiEvent (struct multievent_t &e);
	void processEvent (struct singleevent_t &e);
	void processCommunication (struct comm_t &c);
	void processCommunicator (string &c);
	void processComment (string &c);

	unsigned getNCounterChanges (void)
	  { return nCounterChanges; }

	void dumpSeen (string fnameprefix);
	void dumpMissingValuesIntoPCF (void);
};

Process::Process (string prvFile, bool multievents) : ParaverTrace (prvFile, multievents)
{
	nCounterChanges = 0;

	pcffile = prvFile.substr (0, prvFile.rfind(".prv")) + string (".pcf");

	pcf = new UIParaverTraceConfig;
	try
	{ pcf->parse(pcffile); }
	catch (...)
	{
		cerr << "ERROR! An exception occurred when parsing the PCF file. Check that it exists and it is correctly formatted." << endl;
		exit (-1);
	}

	vector<unsigned> vtypes = pcf->getEventTypes();

	unsigned ncounters = 0;
	for (unsigned u = 0; u < vtypes.size(); u++)
		if ( vtypes[u] >= PAPI_MIN_COUNTER && vtypes[u] <= PAPI_MAX_COUNTER )
			ncounters++;

	numCounterIDs = ncounters;
	CounterIDs = new unsigned long long[numCounterIDs];
	CounterIDNames = new string[numCounterIDs];
	HackCounter = new bool[numCounterIDs];
	CounterUsed = new bool[numCounterIDs];

	for (unsigned u = 0, j = 0; u < vtypes.size(); u++)
		if ( vtypes[u] >= PAPI_MIN_COUNTER && vtypes[u] <= PAPI_MAX_COUNTER )
		{
			bool found;
			string s = getType (vtypes[u], found);
			/* It should always exist because it was returned by getEventTypes... */
			if (found)
			{
				CounterUsed[j] = false;
				CounterIDs[j] = vtypes[u];
				CounterIDNames[j] = s.substr (s.find ('(')+1, s.find (')', s.find ('(')+1) - (s.find ('(') + 1));
				HackCounter[j] = (CounterIDNames[j] == "PM_CMPLU_STALL_FDIV" || CounterIDNames[j] == "PM_CMPLU_STALL_ERAT_MISS")?1:0;
				j++;
			}
		}

	string completefile = prvFile.substr (0, prvFile.length()-4) + ".extract" ;
	IH.outputfile.open ((common::basename(completefile)).c_str());
}

void Process::closeFile (void)
{
	IH.outputfile.close();
}

unsigned Process::LookupCounter (unsigned long long Counter, bool &found)
{
	found = true;

	for (unsigned i = 0; i < numCounterIDs; i++)
		if (CounterIDs[i] == Counter)
			return i;

	found = false;
	return numCounterIDs;
}

void Process::processComment (string &c)
{
	UNREFERENCED(c);
}

void Process::processCommunicator (string &c)
{
	UNREFERENCED(c);
}

void Process::processState (struct state_t &s)
{
	UNREFERENCED(s);
}

void Process::processCaller (struct event_t &evt, unsigned base, Sample &s)
{
	unsigned depth = evt.Type - base;

	if (base == EXTRAE_SAMPLE_CALLER_MIN)
		s.Caller[depth] = evt.Value;
	else if (base == EXTRAE_SAMPLE_CALLERLINE_MIN)
		s.CallerLine[depth] = evt.Value;
	else if (base == EXTRAE_SAMPLE_CALLERLINE_AST_MIN)
		s.CallerLineAST[depth] = evt.Value;
}

void Process::processCounter (struct event_t &evt, Sample &s)
{
	bool found;
	unsigned long long value;
	unsigned index = LookupCounter (evt.Type, found);
    
	if (found) 
	{
		if (HackCounter[index])
		{
			value = evt.Value;
			if (value > 0x80000000LL) /* If counter larger than 32 bits  (2^32) */
			{
				value = value & 0xffffffffLL;
				if ((value & 0x80000000LL) != 0)
					value = value ^ 0xffffffffLL;
				value = 0; // Supersede previous hacking
			}
		}   
		else    
			value = evt.Value;

		s.CounterValues[evt.Type] = value;
	}
}

bool Process::equalTypes (map<unsigned, unsigned long long> &m1,
	map<unsigned, unsigned long long> &m2)
{
	map<unsigned, unsigned long long>::iterator i1 = m1.begin();
	map<unsigned, unsigned long long>::iterator i2 = m2.begin();

	if (m1.size() != m2.size())
		return false;

	/* Check all types in M1 are in M2 in the same order */
	for (; i1 != m1.end(); i1++, i2++)
		if ((*i1).first != (*i2).first)
			return false;

	/* If we are here, then types(M1) = types(M2) */
	return true;
}

bool Process::checkSamples (vector<Sample> &Samples)
{
	/* Triplets are not valid at the edges (first and last) */
	if (Samples.size() > 2)
	{
		map<unsigned, unsigned long long> refCounterValues = Samples[1].CounterValues;

		for (unsigned u = 2; u < Samples.size()-1; u++)
		{
			/* Just check for counter types, that are the same across the instance.
			   We don't need to check for the callers because they may vary from
			   execution to execution (i.e. one sample may get 10 callers but the
			   following can take less). */
			if (!equalTypes (refCounterValues, Samples[u].CounterValues))
			{
				nCounterChanges++;
				return false;
			}
		}
	}

	return true;
}

void Process::dumpSamples (unsigned ptask, unsigned task, unsigned thread,
	unsigned long long region, unsigned long long start,
	unsigned long long duration, vector<Sample> &Samples)
{
	if (common::DEBUG())
		cout << "Process:dumpSamples (Samples.size() = " << Samples.size() << ")" << endl;

	/* At least, have a sample at the begin & end, and someone else */
	if (Samples.size() < 2)
		return;

	if (!checkSamples (Samples))
		return;

	/* Write the total time spent in this region */
	bool RegionFound;
	string RegionName = getTypeValue (RegionSeparator, region, RegionFound);
	if (!RegionFound)
	{
		stringstream ss;
		ss << region;
		RegionName = "Value_" + ss.str();
		IH.addMissingRegion (region);
	}
	RegionName = common::removeSpaces (RegionName);
	IH.addRegion (RegionName);

	/* Calculate totals */
	map<unsigned, unsigned long long> totals;
	map<unsigned, unsigned long long>::iterator it = Samples[0].CounterValues.begin();
	for (; it != Samples[0].CounterValues.end(); it++)
		totals[(*it).first] = (*it).second;

	/* Generate header for this instance */
	IH.outputfile << "I " << ptask+1 << " " << task+1 << " " << thread+1 
	  << " " << RegionName << " " << start << " " << duration;
	for (unsigned u = 1; u < Samples.size(); u++)
	{
		it = Samples[u].CounterValues.begin();
		for (; it != Samples[u].CounterValues.end(); it++)
			totals[(*it).first] = totals[(*it).first] + (*it).second;
	}
	IH.outputfile << " " << totals.size();
	map<unsigned, unsigned long long>::iterator it_totals = totals.begin();
	for (; it_totals != totals.end(); it_totals++)
	{
		bool found;
		unsigned index = LookupCounter ((*it_totals).first, found);
		if (!found)
			cout << "Error! Cannot find counter " << (*it_totals).first << endl;
		IH.outputfile << " " << CounterIDNames[index] << " " << (*it_totals).second;
		IH.addCounter (CounterIDNames[index]);
	}
	IH.outputfile << endl;

	/* Do not emit samples if we only have a sample at the end */
	if (Samples[0].Timestamp == start + duration)
		return;

	/* Prepare partial accum hash */
	map<unsigned, unsigned long long> partials;
	for (it = Samples[0].CounterValues.begin();
	     it != Samples[0].CounterValues.end();
	     it++)
		partials[(*it).first] = 0;

	/* Dump all the samples for this instance except the last one, which can be extrapolated
       from the instance header data */
	for (unsigned u = 0; u < Samples.size() - 1; u++)
	{
		IH.outputfile << "S " << Samples[u].Timestamp << " " << Samples[u].Timestamp - start;

		IH.outputfile << " " << Samples[u].CounterValues.size();
		it = Samples[u].CounterValues.begin();
		for (; it != Samples[u].CounterValues.end(); it++)
		{
			bool found;
			unsigned index = LookupCounter ((*it).first, found);
			if (!found)
				cout << "Error! Cannot find counter " << (*it).first << endl;

			unsigned long long tmp = partials[(*it).first] + (*it).second;
			IH.outputfile << " " << CounterIDNames[index] << " " << tmp;
			partials[(*it).first] = tmp;
		}

		/* Dump callers, callerlines and caller line ASTs triplets */
		IH.outputfile << " " << Samples[u].Caller.size();

		map<unsigned, unsigned long long>::iterator it1 = Samples[u].Caller.begin();
		map<unsigned, unsigned long long>::iterator it2 = Samples[u].CallerLine.begin();
		map<unsigned, unsigned long long>::iterator it3 = Samples[u].CallerLineAST.begin();
		for (; it1 != Samples[u].Caller.end(); it1++, it2++, it3++)
			IH.outputfile << " " << (*it1).first << " " << 
			  (*it1).second << " " << (*it2).second << " " <<
			  (*it3).second;

		IH.outputfile << endl;
	}

	Samples.clear();
}

void Process::processMultiEvent (struct multievent_t &e)
{
	bool FoundSeparator = false;
	bool FoundPhaseSeparator = false;
	unsigned long long ValueSeparator = 0;
	unsigned ptask = e.ObjectID.ptask - 1;
	unsigned task = e.ObjectID.task - 1;
	unsigned thread = e.ObjectID.thread - 1;

	if (ptask >= IH.getNumPTasks())
		return;

	PTaskInformation *pti = IH.getPTasksInformation();
	if (task >= pti[ptask].getNumTasks())
		return;

	TaskInformation *ti = pti[ptask].getTasksInformation();
	if (thread >= ti[task].getNumThreads())
		return;

	ThreadInformation *thi = ti[task].getThreadsInformation();

	bool storeSample = false;
	Sample s;
	s.Timestamp = e.Timestamp;

	for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
	{
		if (thi[thread].CurrentRegion > 0)
		{
			if ((*it).Type >= PAPI_MIN_COUNTER && (*it).Type <= PAPI_MAX_COUNTER )
			{
				if (common::DEBUG())
					cout << "Processing counter " << (*it).Type << " at timestamp " << e.Timestamp << endl;

				processCounter (*it, s);
				storeSample = true;
			}
			if ((*it).Type >= EXTRAE_SAMPLE_CALLER_MIN && (*it).Type <= EXTRAE_SAMPLE_CALLER_MAX)
			{
				if (common::DEBUG())
					cout << "Processing C " << (*it).Type << " at timestamp " << e.Timestamp << endl;

				processCaller (*it, EXTRAE_SAMPLE_CALLER_MIN, s);
				storeSample = true;
			}
			if ((*it).Type >= EXTRAE_SAMPLE_CALLERLINE_MIN && (*it).Type <= EXTRAE_SAMPLE_CALLERLINE_MAX)
			{
				if (common::DEBUG())
					cout << "Processing CL " << (*it).Type << " at timestamp " << e.Timestamp << endl;

				processCaller (*it, EXTRAE_SAMPLE_CALLERLINE_MIN, s);
				storeSample = true;
			}
			if ((*it).Type >= EXTRAE_SAMPLE_CALLERLINE_AST_MIN && (*it).Type <= EXTRAE_SAMPLE_CALLERLINE_AST_MAX)
			{
				if (common::DEBUG())
					cout << "Processing CL-AST " << (*it).Type << " at timestamp " << e.Timestamp << endl;

				processCaller (*it, EXTRAE_SAMPLE_CALLERLINE_AST_MIN, s);
				storeSample = true;
			}
		}

		if ((*it).Type == RegionSeparator)
		{
			if (common::DEBUG())
				cout << "Found separator (" << RegionSeparator << "," << (*it).Value << ") at timestamp " << e.Timestamp << endl;

			FoundSeparator = true;
			ValueSeparator = (*it).Value;
		}
	}

	if (common::DEBUG())
		cout << "storeSample = " << storeSample << " at timestamp = " << e.Timestamp << endl;

	if (storeSample && !(FoundSeparator && ValueSeparator > 0))
	{
		if (common::DEBUG())
		{
			map<unsigned, unsigned long long>::iterator it = s.CounterValues.begin();
			for (; it != s.CounterValues.end(); it++)
				cout << " " << (*it).first << " " << (*it).second;
			cout << endl;
		}
		thi[thread].Samples.push_back (s);
	}
	else
	{
		if (common::DEBUG())
			cout << "not adding sample because it is the start of a region!" << endl;
	}

	/* If we found a region separator, increase current region and reset the phase */
	if (FoundSeparator)
	{
		if (thi[thread].CurrentRegion > 0)
		{
			dumpSamples (ptask, task, thread, thi[thread].CurrentRegion,
			  thi[thread].StartRegion, e.Timestamp - thi[thread].StartRegion,
			  thi[thread].Samples);
			thi[thread].Samples.clear();
			thi[thread].setSeen (true);
		}

		thi[thread].CurrentRegion = ValueSeparator;
		thi[thread].StartRegion = e.Timestamp;
	}
}

void Process::processEvent (struct singleevent_t &e)
{
	UNREFERENCED(e);
}

void Process::processCommunication (struct comm_t &c)
{
	UNREFERENCED(c);
}

void Process::allocateBuffers (void)
{
	vector<ParaverTraceApplication *> va = this->get_applications();

	if (common::DEBUG())
		cout << "Application has " << va.size() << " ptasks" << endl;

	IH.AllocatePTasks (va.size());
	PTaskInformation *pti = IH.getPTasksInformation();
	for (unsigned ptask = 0; ptask < va.size(); ptask++)
	{
		vector<ParaverTraceTask *> vt = va[ptask]->get_tasks();

		if (common::DEBUG())
			cout << " Ptask " << ptask+1 << " contains " << vt.size() << " tasks" << endl;

		pti[ptask].AllocateTasks (vt.size());
		TaskInformation *ti = pti->getTasksInformation();
		for (unsigned int task = 0; task < vt.size(); task++)
		{
			unsigned nthreads = vt[task]->get_threads().size(); /* o vt[task]->get_threads()[0]->get_key() ? */

			if (common::DEBUG())
				cout << "  Task " << task+1 << " contains " << nthreads << " threads" << endl;

			ti[task].AllocateThreads (nthreads);
		}		
	}

}

string Process::getType (unsigned type, bool &found)
{
	found = true;
	string s;

	try
	{ s = pcf->getEventType (type); }
	catch (...)
	{
		cerr << "Warning! Did not find the description of type " << type << " in the PCF file... Will add the definition" << endl; 
		found = false;
		s = "";
	}

	return s;
}

string Process::getTypeValue (unsigned type, unsigned value, bool &found)
{
	found = true;
	string s;

	try
	{ s = pcf->getEventValue (type, value); }
	catch (...)
	{
		found = false;
		s = "";
	}

	return s;
}

void Process::dumpSeenObjects (string filename)
{
	ofstream f ((common::basename(filename.c_str())).c_str());
	if (f.is_open())
	{
		PTaskInformation *ptaskinfo = IH.getPTasksInformation();
		for (unsigned ptask = 0; ptask < IH.getNumPTasks(); ptask++)
		{
			TaskInformation *taskinfo = ptaskinfo[ptask].getTasksInformation();
			for (unsigned task = 0; task < ptaskinfo[ptask].getNumTasks(); task++)
			{
				ThreadInformation *threadinfo = taskinfo[task].getThreadsInformation();
				for (unsigned thread = 0; thread < taskinfo[task].getNumThreads(); thread++)
					if (threadinfo[thread].getSeen())
						f << ptask+1 << "." << task+1 << "." << thread+1 << endl;
			} 
		}
	}
	f.close ();
}

void Process::dumpSeenCounters (string filename)
{
	ofstream f (filename.c_str());
	if (f.is_open())
	{
		set<string>::iterator i;
		set<string> ctrs = IH.getCounters ();
		for (i = ctrs.begin(); i != ctrs.end(); i++)
			f << *i << endl;
	}
	f.close();
}

void Process::dumpSeenRegions (string filename)
{
	ofstream f (filename.c_str());
	if (f.is_open())
	{
		set<string>::iterator i;
		set<string> ctrs = IH.getRegions ();
		for (i = ctrs.begin(); i != ctrs.end(); i++)
			f << *i << endl;
	}
	f.close();
}

void Process::dumpSeen (string fnameprefix)
{
	dumpSeenObjects (fnameprefix+".objects");
	dumpSeenCounters (fnameprefix+".counters");
	dumpSeenRegions (fnameprefix+".regions");
}

void Process::dumpMissingValuesIntoPCF (void)
{
	set<unsigned> m = IH.getMissingRegions();
	if (m.size() > 0)
	{
		fstream f (pcffile.c_str(), fstream::out | fstream::app);
		if (f.is_open())
		{
			f << "EVENT_TYPE" << endl
			  << "0 " << RegionSeparator << " " << RegionSeparatorName << endl
			  << "VALUES" << endl;
			set<unsigned>::iterator i;
			for (i = m.begin(); i != m.end(); i++)
			{
				cout << "Warning! Adding missing region label for type " << RegionSeparator
				  << " value " << *i << " into " << pcffile << endl;
				stringstream ss;
				ss << *i;
				f << *i << " Value_" << ss.str() << endl;
			}
		}
		f.close();
	}
}

} /* namespace libparaver */

using namespace::libparaver;
using namespace::std;

int ProcessParameters (int argc, char *argv[])
{
	if (argc < 2)
	{
		cerr << "Insufficient number of parameters" << endl
		     << "Available options are: " << endl
		     << "-separator S" << endl;
		exit (-1);
	}

	for (int i = 1; i < argc-1; i++)
	{
		if (strcmp ("-separator",  argv[i]) == 0)
		{
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid number of separator" << endl;
			else 
				RegionSeparator = atoi(argv[i]);
			continue;
		}
	}

	return argc-1;
}

int main (int argc, char *argv[])
{
	int res = ProcessParameters (argc, argv);

	if (RegionSeparator == 0)
	{
		cerr << "Error! Please, provide a valid separator for -separator parameter" << endl;
		exit (-1);
	}

	string tracename = string(argv[res]);
	if (!common::existsFile(tracename))
	{
		cerr << "The tracefile " << tracename << " does not exist!" << endl;
		return -2;
	}

	Process *p;
	try
	{ p = new Process (tracename, true); }
	catch (...)
	{
		cerr << "ERROR! Exception launched when processing the file " << tracename << ". Check that it exists and it is a Paraver tracefile..." << endl; 
		return -1;
	}

	bool found;
	cout << "Checking for basic caller information" << flush;
	string t1 = p->getType (EXTRAE_SAMPLE_CALLER_MIN, found);
	if (!found)
	{
		cerr << endl << "Unable to get caller information (event type " << EXTRAE_SAMPLE_CALLER_MIN << ")" << endl;
		exit (-1);
	}
	cout << ", caller line information" << flush;
	string t2 = p->getType (EXTRAE_SAMPLE_CALLERLINE_MIN, found);
	if (!found)
	{
		cerr << endl << "Unable to get caller line information (event type " << EXTRAE_SAMPLE_CALLERLINE_MIN << ")" << endl;
		exit (-1);
	}
	cout << ", caller line AST information" << flush;
	string t3 = p->getType (EXTRAE_SAMPLE_CALLERLINE_AST_MIN, found);
	cout << " Done" << endl;
	if (!found)
	{
		cerr << endl << "Unable to get caller line AST information (event type " << EXTRAE_SAMPLE_CALLERLINE_AST_MIN << ")" << endl;
		exit (-1);
	}

	/* By loading these t1-3 we ensure that the all callers triplets exist */
	bool RegionSeparatorFound;
	RegionSeparatorName = p->getType (RegionSeparator, RegionSeparatorFound);
	if (!RegionSeparatorFound)
	{
		stringstream ss;
		ss << RegionSeparator;
		RegionSeparatorName = "Event_" + ss.str();
	}
	RegionSeparatorName = common::removeSpaces (RegionSeparatorName);

	cout << "Extracting data for type " << RegionSeparator << " (" << RegionSeparatorName << ")" << endl;

	p->allocateBuffers ();
	p->parseBody();
	p->closeFile();

	p->dumpSeen (common::basename (tracename.substr (0, tracename.length()-4)));
	p->dumpMissingValuesIntoPCF ();

	if (p->getNCounterChanges() > 0)
		cout << "Ignored " << p->getNCounterChanges() << " instances, most probably because of hardware counter set change." << endl;

	string ControlFile = common::basename (tracename.substr (0, tracename.length()-4) + ".control");
	ofstream cfile (ControlFile.c_str());
	cfile << tracename << endl;
	cfile << RegionSeparator << endl;
	cfile << RegionSeparatorName << endl;
	cfile.close ();

	return 0;
}

