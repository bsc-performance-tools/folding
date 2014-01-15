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

#include "ParaverTrace.h"
#include "ParaverTraceThread.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceApplication.h"
#include "UIParaverTraceConfig.h"
#include "prv-semantic-CSV.H"

#include <sstream>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <list>
#include <set>
#include <exception>

#include "sample.H"
#include "folding-writer.H"
#include "prv-types.H"

unsigned long long RegionSeparator = 0;
string RegionSeparatorName;
string PRVSemanticCSVName;

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


class ThreadInformation
{
	private:
	bool seen;

	public:
	vector<Sample*> Samples;

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

class AddressReference
{
	private:
	unsigned long long address;
	bool has_address;
	unsigned reference_mem_level;
	bool has_reference_mem_level;
	unsigned reference_tlb_level;
	bool has_reference_tlb_level;

	public:
	AddressReference ()
	  { has_address = has_reference_mem_level = has_reference_tlb_level = false; }
	void setAddress (unsigned long long Address)
	  { address = Address; has_address = true; }
	void setReferenceMemLevel (unsigned MemLevel)
	  { reference_mem_level = MemLevel; has_reference_mem_level = true; }
	void setReferenceTLBLevel (unsigned TLBLevel)
	  { reference_tlb_level = TLBLevel; has_reference_tlb_level = true; }
	bool isCompleted (void) const
	  { return has_address && has_reference_mem_level && has_reference_tlb_level; }
	unsigned long long getAddress (void) const
	  { return address; }
	unsigned getReferenceMemLevel (void) const
	  { return reference_mem_level; }
	unsigned getReferenceTLBLevel (void) const
	  { return reference_tlb_level; }
};

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
	PRVSemanticCSV *Semantics;

	bool TimeOffsetSet;	 // Specific to CSV support
	unsigned long long TimeOffset; // Specific to CSV support
	vector<string> SemanticIndex; // Specific to CSV support

	void processCaller (const struct event_t &evt, unsigned base,
	  map<unsigned, unsigned long long> &C);
	void processCounter (const struct event_t &evt,
	  map<string, unsigned long long> &m);
	void processAddressReference (const struct event_t &evt,
	  AddressReference &ar);

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

	unsigned getNCounterChanges (void) const
	  { return nCounterChanges; }

	bool givenSemanticsCSV(void) const
	  { return Semantics != NULL; }

	void dumpSeen (string fnameprefix);
	void dumpMissingValuesIntoPCF (void);
};

Process::Process (string prvFile, bool multievents) : ParaverTrace (prvFile, multievents)
{
	TimeOffsetSet = false;
	nCounterChanges = 0;

	if (PRVSemanticCSVName.length() > 0)
	{
		Semantics = new PRVSemanticCSV (PRVSemanticCSVName.c_str());
		if (Semantics == NULL)
		{
			cerr << "Cannot allocate memory for semantic parser" << endl;
			exit (-1);
		}
		else
		{
			if (!Semantics->getSucceeded())
			{
				cerr << "An error ocurred while parsing the semantic file" << endl;
				exit (-1);
			}
		}
	}

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
	if (Semantics != NULL && !TimeOffsetSet)
	{
		if (c.find ("Cut time range") != string::npos)
			// Ensure that Offset and from are there
			if (c.find ("Offset ") != string::npos && c.find (" from ") != string::npos)
			{
				string tmp = c.substr (c.find ("Offset ") + strlen ("Offset "), c.find(" from ") - c.find ("Offset "));
				TimeOffset = atoll (tmp.c_str());
			}
	}
}

void Process::processCommunicator (string &c)
{
	UNREFERENCED(c);
}

void Process::processState (struct state_t &s)
{
	UNREFERENCED(s);
}

void Process::processAddressReference (const struct event_t &evt,
	  AddressReference &ar)
{
	if (evt.Type == EXTRAE_SAMPLE_ADDRESS)
		ar.setAddress (evt.Value);
	else if (evt.Type == EXTRAE_SAMPLE_ADDRESS_MEM_LEVEL)
		ar.setReferenceMemLevel (evt.Value);
	else if (evt.Type == EXTRAE_SAMPLE_ADDRESS_TLB_LEVEL)
		ar.setReferenceTLBLevel (evt.Value);
}

void Process::processCaller (const struct event_t &evt, unsigned base,
	map<unsigned, unsigned long long> &C)
{
	unsigned depth = evt.Type - base;
	C[depth] = evt.Value;
}

void Process::processCounter (const struct event_t &evt,
	map<string, unsigned long long> &m)
{
	bool found;
	unsigned long long value;
	unsigned index = LookupCounter (evt.Type, found);
    
	if (found) 
	{
		IH.addCounter (CounterIDNames[index]);

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

		m[CounterIDNames[index]] = value;
	}
}

void Process::processMultiEvent (struct multievent_t &e)
{
	bool FoundSeparator = false;
	unsigned long long ValueSeparator = 0;
	int ptask = e.ObjectID.ptask - 1;
	int task = e.ObjectID.task - 1;
	int thread = e.ObjectID.thread - 1;

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

	AddressReference AR;                             /* Address reference info */
	map<string, unsigned long long> CV;              /* Map of Counters and their Values */
	map<unsigned, unsigned long long> Caller;        /* Map depth of caller */
	map<unsigned, unsigned long long> CallerLine;    /* Map depth of caller line */
	map<unsigned, unsigned long long> CallerLineAST; /* Map depth of caller line AST */

	for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
	{
		if (thi[thread].CurrentRegion > 0)
		{
			if ((*it).Type >= PAPI_MIN_COUNTER && (*it).Type <= PAPI_MAX_COUNTER )
			{
				if (common::DEBUG())
					cout << "Processing counter " << (*it).Type << " at timestamp " << e.Timestamp << endl;

				processCounter (*it, CV);
				storeSample = true;
			}
			if ((*it).Type >= EXTRAE_SAMPLE_CALLER_MIN && (*it).Type <= EXTRAE_SAMPLE_CALLER_MAX)
			{
				if (common::DEBUG())
					cout << "Processing C " << (*it).Type << " at timestamp " << e.Timestamp << endl;

				processCaller (*it, EXTRAE_SAMPLE_CALLER_MIN, Caller);
				storeSample = true;
			}
			if ((*it).Type >= EXTRAE_SAMPLE_CALLERLINE_MIN && (*it).Type <= EXTRAE_SAMPLE_CALLERLINE_MAX)
			{
				if (common::DEBUG())
					cout << "Processing CL " << (*it).Type << " at timestamp " << e.Timestamp << endl;

				processCaller (*it, EXTRAE_SAMPLE_CALLERLINE_MIN, CallerLine);
				storeSample = true;
			}
			if ((*it).Type >= EXTRAE_SAMPLE_CALLERLINE_AST_MIN && (*it).Type <= EXTRAE_SAMPLE_CALLERLINE_AST_MAX)
			{
				if (common::DEBUG())
					cout << "Processing CL-AST " << (*it).Type << " at timestamp " << e.Timestamp << endl;

				processCaller (*it, EXTRAE_SAMPLE_CALLERLINE_AST_MIN, CallerLineAST);
				storeSample = true;
			}
			if ((*it).Type == EXTRAE_SAMPLE_ADDRESS || 
			    (*it).Type == EXTRAE_SAMPLE_ADDRESS_MEM_LEVEL || 
			    (*it).Type == EXTRAE_SAMPLE_ADDRESS_TLB_LEVEL )
			{
				processAddressReference (*it, AR);
				storeSample = true;
			}
		}

		if (Semantics == NULL)
		{
			if ((*it).Type == RegionSeparator)
			{
				if (common::DEBUG())
					cout << "Found separator (" << RegionSeparator << "," << (*it).Value << ") at timestamp " << e.Timestamp << endl;

				FoundSeparator = true;
				ValueSeparator = (*it).Value;
			}
		}
	}

	if (Semantics != NULL)
	{
		vector<PRVSemanticValue *> vs = Semantics->getSemantics (e.ObjectID.ptask, e.ObjectID.task, e.ObjectID.thread);
		for (unsigned sem = 0; sem < vs.size(); sem++)
		{
			if (vs[sem]->getFrom() == e.Timestamp + TimeOffset)
			{
				FoundSeparator = true;
				string tmp = vs[sem]->getValue().substr (0, vs[sem]->getValue().find("."));
				ValueSeparator = atoi (tmp.c_str());
				if (vs[sem]->getValue() != "End")
				{
					ValueSeparator = 0;
					for (unsigned i = 0; i < SemanticIndex.size(); i++)
						if (SemanticIndex[i] == vs[sem]->getValue())
						{
							ValueSeparator = i+1;
							break;
						}
					if (ValueSeparator == 0)
					{
						SemanticIndex.push_back (vs[sem]->getValue());
						ValueSeparator = SemanticIndex.size();
					}
				}
				else
					ValueSeparator = 0;

				if (common::DEBUG())
					cout << "Found semantic separator value " << vs[sem]->getValue()
					  << " [" << ValueSeparator << "] at timestamp " << e.Timestamp
					  << endl;
				break;
			}
		}
	}

	if (common::DEBUG())
		cout << "storeSample = " << storeSample << " at timestamp = " << e.Timestamp << endl;

	if (storeSample && !(FoundSeparator && ValueSeparator > 0))
	{
		map<unsigned, unsigned long long>::iterator cit;
		map<unsigned, CodeRefTriplet> CodeRefs;
		for (cit = Caller.begin(); cit != Caller.end(); ++cit)
		{
			unsigned d = (*cit).first;

			assert (Caller.count(d) == 1);
			assert (CallerLine.count(d) == 1);
			assert (CallerLineAST.count(d) == 1);

			CodeRefTriplet t (Caller[d], CallerLine[d],
			  CallerLineAST[d]);
			CodeRefs[d] = t;
		}

		Sample *s;
		if (AR.isCompleted())
			s = new Sample (e.Timestamp, e.Timestamp - thi[thread].StartRegion, CV, CodeRefs,
			  AR.getAddress(), AR.getReferenceMemLevel(), AR.getReferenceTLBLevel());
		else
			s = new Sample (e.Timestamp, e.Timestamp - thi[thread].StartRegion, CV, CodeRefs);
		assert (s != NULL);

		if (common::DEBUG())
		{
			map<string, unsigned long long>::iterator it;
			for (it = CV.begin(); it != CV.end(); it++)
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
		unsigned long long Region = thi[thread].CurrentRegion;
		if (Region > 0)
		{
			/* Look for the region name */
			string RegionName;
			if (Semantics == NULL)
			{
				bool RegionFound;
				RegionName = getTypeValue (RegionSeparator, Region, RegionFound);
				if (!RegionFound)
				{
					stringstream ss;
					ss << Region;
					RegionName = "Value_" + ss.str();
					IH.addMissingRegion (Region);
				}
				RegionName = common::removeSpaces (RegionName);
				IH.addRegion (RegionName);
			}
			else
				RegionName = SemanticIndex[Region-1];

			/* Write the information */

			FoldingWriter::Write (IH.outputfile, RegionName, ptask, task,
			  thread, thi[thread].StartRegion,
			  e.Timestamp - thi[thread].StartRegion,
			  thi[thread].Samples);

			/* Clean */

			for (unsigned s = 0; s < thi[thread].Samples.size(); ++s)
				delete thi[thread].Samples[s];

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
		for (int ptask = 0; ptask < IH.getNumPTasks(); ptask++)
		{
			TaskInformation *taskinfo = ptaskinfo[ptask].getTasksInformation();
			for (int task = 0; task < ptaskinfo[ptask].getNumTasks(); task++)
			{
				ThreadInformation *threadinfo = taskinfo[task].getThreadsInformation();
				for (int thread = 0; thread < taskinfo[task].getNumThreads(); thread++)
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
		set<string> regions = IH.getRegions ();
		for (i = regions.begin(); i != regions.end(); i++)
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
		     << "-separator S" << endl
		     << "-semantic F" << endl;
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
		else if (strcmp ("-semantic", argv[i]) == 0)
		{
			i++;
			if (common::existsFile(argv[i]))
			{
				cout << "Reading semantic file " << argv[i] << endl;
				PRVSemanticCSVName = argv[i];
			}
			else
			{
				cerr << "The file " << argv[i] << " does exist. Dying ... " << endl;
				exit (-1);
			}
			continue;
		}
	}

	return argc-1;
}

int main (int argc, char *argv[])
{
	cout << "Folding (extract) based on branch " FOLDING_SVN_BRANCH " revision " << FOLDING_SVN_REVISION << endl;

	int res = ProcessParameters (argc, argv);

	if (RegionSeparator == 0 && PRVSemanticCSVName.length() == 0)
	{
		cerr << "Error! Please, provide a valid separator for -separator parameter or a semantic CSV file through -semantic" << endl;
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
	if (!p->givenSemanticsCSV())
	{
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
	}
	else
		cout << "Extracting data for semantic values" << endl;

	p->allocateBuffers ();
	p->parseBody();
	p->closeFile();

	p->dumpSeen (common::basename (tracename.substr (0, tracename.length()-4)));

	// Emit seen event types when passing a separator manually
	if (!p->givenSemanticsCSV())
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

