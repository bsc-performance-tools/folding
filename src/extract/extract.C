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
#include "data-object.H"
#include "pcf-common.H"

static string RegionSeparator;
static unsigned long long RegionSeparatorID = 0;
static string RegionSeparatorName;
static string PRVSemanticCSVName;
static int max_callstack_depth = 0;
static bool has_max_callstack_depth = false;
static bool has_malloc_level = false;
static unsigned malloc_level = 0;

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
	vector<Sample*> Samples;
	unsigned long long CurrentRegion;
	unsigned long long StartRegion;

	public:
	unsigned long long getCurrentRegion(void) const
	  { return CurrentRegion; }
	void setCurrentRegion (unsigned long long v)
	  { CurrentRegion = v; }
	unsigned long long getStartRegion(void) const
	  { return StartRegion; }
	void setStartRegion (unsigned long long v)
	  { StartRegion = v; }
	bool getSeen (void) const
	  { return seen; }
	void setSeen (bool b)
	  { seen = b; }
	void addSample (Sample *s)
	  { Samples.push_back (s); }
	const vector<Sample*> & getSamples (void) const
	  { return Samples; }
	void clearSamples (void)
	  {
		for (auto s : Samples) 
			delete s;
		Samples.clear();
	  }
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
	vector<DataObject*> dataObjects;
	DataObject_dynamic *tmpdataobject;
	set<unsigned> livingDataObjects;

	public:
	int getNumThreads (void) const
	{ return numThreads; };

	ThreadInformation* getThreadsInformation (void) const
	{ return ThreadsInfo; };

	void AllocateThreads (int nThreads);
	~TaskInformation();

	void addDataObject (DataObject *_do)
	  {
	    dataObjects.push_back (_do);
		livingDataObjects.insert (dataObjects.size()-1);
	  }
	void removeDataObject (unsigned long long address)
	  {
		/* We want to reference every allocated memory region, so we don't
		   actually remove the data object reference. We only remove its
		   last living reference */
	    for (size_t i = 0; i < dataObjects.size(); i++)
			if (dataObjects[i]->getStartAddress() == address)
				if (livingDataObjects.count (i) > 0)
				{
					livingDataObjects.erase (i);
					break;
				}
	  }
	const vector<DataObject*> & getDataObjects (void) const
	  {  return dataObjects; }
	void setTmpDataObject (DataObject_dynamic *_do)
	  { tmpdataobject = _do; }
	DataObject_dynamic * getTmpDataObject (void) const
	  { return tmpdataobject; } 
	set<unsigned> getLivingDataObjects (void) const
	  { return livingDataObjects; }
};

TaskInformation::~TaskInformation()
{
	delete [] ThreadsInfo;
}

void TaskInformation::AllocateThreads (int nThreads)
{
	numThreads = nThreads;
	ThreadsInfo = new ThreadInformation[nThreads];
}

class PTaskInformation
{
	private:
	int numTasks;
	TaskInformation *TasksInfo;

	public:
	int getNumTasks (void) const
	{ return numTasks; };

	TaskInformation* getTasksInformation (void) const
	{ return TasksInfo; };

	void AllocateTasks (int nTakss);
	~PTaskInformation();
};

PTaskInformation::~PTaskInformation()
{
	delete [] TasksInfo;
}

void PTaskInformation::AllocateTasks (int nTasks)
{
	numTasks = nTasks;
	TasksInfo = new TaskInformation[nTasks];
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

	PTaskInformation* getPTasksInformation (void) const
	{ return PTasksInfo; };

	void AllocatePTasks (int numPTasks);
	~InformationHolder();

	void addCounter (string c)
	  { seenCounters.insert (c); }
	void addRegion (string r)
	  { seenRegions.insert (r); }
	void addMissingRegion (unsigned r)
	  { missingRegions.insert (r); }
	const set<string> & getCounters (void) const
	  { return seenCounters; }
	const set<string> & getRegions (void) const
	  { return seenRegions; }
	const set<unsigned> & getMissingRegions (void) const
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
	unsigned core_cycles;
	bool has_core_cycles;
	AddressReferenceType_t ReferenceType;

	public:
	AddressReference ()
	  { has_address = has_reference_mem_level = has_reference_tlb_level = false; }
	void setAddress (unsigned long long Address)
	  { address = Address; has_address = true; }
	void setReferenceMemLevel (unsigned MemLevel)
	  { reference_mem_level = MemLevel; has_reference_mem_level = true; }
	void setReferenceTLBLevel (unsigned TLBLevel)
	  { reference_tlb_level = TLBLevel; has_reference_tlb_level = true; }
	void setReferenceCoreCycles (unsigned cycles)
	  { core_cycles = cycles; has_core_cycles = true; }
	void setReferenceType (AddressReferenceType_t t)
	  { ReferenceType = t; }
	bool isCompleted (void) const
	  { return has_address && has_reference_mem_level && has_reference_tlb_level; }
	bool hasAddress (void) const
	  { return has_address; }
	unsigned long long getAddress (void) const
	  { return address; }
	unsigned getReferenceMemLevel (void) const
	  { return reference_mem_level; }
	unsigned getReferenceTLBLevel (void) const
	  { return reference_tlb_level; }
	unsigned setReferenceCoreCycles (void) const
	  { return core_cycles; }
	AddressReferenceType_t getReferenceType (void) const
	  { return ReferenceType; }
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

	bool processCaller (const struct event_t &evt, unsigned base,
	  map<unsigned, unsigned long long> &C);
	void processCounter (const struct event_t &evt,
	  map<string, unsigned long long> &m);
	void processAddressReference (const struct event_t &evt,
	  AddressReference &ar);

	void dumpSeenObjects (string fnameprefix);
	void dumpSeenCounters (string fnameprefix);
	void dumpSeenRegions (string fnameprefix);
	void dumpSeenAddressRegions (string filename);

	public:
	Process (string prvFile, bool multievents);

	unsigned lookupType (const string &type);
	string getType (unsigned type, bool &found, bool silent = true);
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
	bool anySeenObjects (void) const;
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

	unsigned ncounters = 0;
	for (const auto type : pcf->getEventTypes())
		if ( type >= PAPI_MIN_COUNTER && type <= PAPI_MAX_COUNTER )
			ncounters++;

	numCounterIDs = ncounters;
	CounterIDs = new unsigned long long[numCounterIDs];
	CounterIDNames = new string[numCounterIDs];
	HackCounter = new bool[numCounterIDs];
	CounterUsed = new bool[numCounterIDs];

	unsigned j = 0;
	for (const auto type : pcf->getEventTypes())
		if ( type >= PAPI_MIN_COUNTER && type <= PAPI_MAX_COUNTER )
		{
			bool found;
			string s = getType (type, found);
			/* It should always exist because it was returned by getEventTypes... */
			if (found)
			{
				CounterUsed[j] = false;
				CounterIDs[j] = type;
				string tmp = s.substr (0, s.find (' '));
				if (tmp[0] == '(' && tmp[tmp.length()-1] == ')')
					CounterIDNames[j] = tmp.substr (1, tmp.length()-2);
				else
					CounterIDNames[j] = tmp;
				HackCounter[j] = (CounterIDNames[j] == "PM_CMPLU_STALL_FDIV" || CounterIDNames[j] == "PM_CMPLU_STALL_ERAT_MISS")?true:false;
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

unsigned Process::lookupType (const string &type_str)
{
	for (const auto type : pcf->getEventTypes())
	{
		bool found;
		string s = getType (type, found);
		if (found)
			if (type_str == s)
				return type;
	}
	return 0;
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
	if (evt.Type == EXTRAE_SAMPLE_ADDRESS_LD)
	{
		ar.setAddress (evt.Value);
		ar.setReferenceType (LOAD);
	}
	else if (evt.Type == EXTRAE_SAMPLE_ADDRESS_ST)
	{
		ar.setAddress (evt.Value);
		ar.setReferenceType (STORE);
	}
	else if (evt.Type == EXTRAE_SAMPLE_ADDRESS_MEM_LEVEL)
		ar.setReferenceMemLevel (evt.Value);
	else if (evt.Type == EXTRAE_SAMPLE_ADDRESS_TLB_LEVEL)
		ar.setReferenceTLBLevel (evt.Value);
	else if (evt.Type == EXTRAE_SAMPLE_ADDRESS_REFERENCE_CYCLES)
		ar.setReferenceCoreCycles (evt.Value);
}

bool Process::processCaller (const struct event_t &evt, unsigned base,
	map<unsigned, unsigned long long> &C)
{
	unsigned depth = evt.Type - base;

	if (!has_max_callstack_depth ||
	    (has_max_callstack_depth && depth <= max_callstack_depth))
	{
		C[depth] = evt.Value;
		return true;
	}
	else
		return false;
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
	bool hasValueSeparator_Start = false;
	bool hasValueSeparator_End   = false;
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

	/* Is there a memory-related call? */
	bool memory_call_alloc = false, memory_call_free = false;
	for (const auto & event : e.events)
		if (event.Type == EXTRAE_DYNAMIC_MEMORY_TYPE)
			if (event.Value == EXTRAE_DYNAMIC_MEMORY_MALLOC ||
			    event.Value == EXTRAE_DYNAMIC_MEMORY_CALLOC ||
			    event.Value == EXTRAE_DYNAMIC_MEMORY_REALLOC)
			{
				memory_call_alloc = true;
				break;
			}
			else if (event.Value == EXTRAE_DYNAMIC_MEMORY_FREE)
			{
				memory_call_free = true;
				break;
			}

	if (memory_call_alloc)
	{
		bool has_minimum_call = false;
		unsigned minimum_call_type;
		unsigned minimum_call_value;
		unsigned long long dynamic_memory_size = 0;
		for (const auto & event : e.events)
		{
			if (event.Type == EXTRAE_DYNAMIC_MEMORY_SIZE)
				dynamic_memory_size = event.Value;

			if (has_malloc_level)
			{
				if (event.Type == malloc_level)
				{
					minimum_call_type = event.Type;
					minimum_call_value = event.Value;
					has_minimum_call = true;
				}
			}
			else if (event.Type >= EXTRAE_CALLERLINE_MIN && event.Type <= EXTRAE_CALLERLINE_MAX)
			{
				if (has_minimum_call && event.Type <= minimum_call_type)
				{
					minimum_call_type = event.Type;
					minimum_call_value = event.Value;
				}
				else if (!has_minimum_call)
				{
					minimum_call_type = event.Type;
					minimum_call_value = event.Value;
					has_minimum_call = true;
				}
			}
		}
		if (dynamic_memory_size != 0 && has_minimum_call)
		{
			bool found = false;
			string name, tmp = getTypeValue (minimum_call_type, minimum_call_value, found);
			if (found)
				name = common::removeUnwantedChars (tmp.substr (0, tmp.find (')')));
			DataObject_dynamic *d = new DataObject_dynamic (dynamic_memory_size,
			  found?name:string("<unknown>"));
			ti[task].setTmpDataObject (d);
		}
	}
	else if (memory_call_free)
	{
		unsigned long long dynamic_memory_inptr = 0;
		for (const auto & event : e.events)
			if (event.Type == EXTRAE_DYNAMIC_MEMORY_IN_PTR)
				dynamic_memory_inptr = event.Value;

		if (dynamic_memory_inptr)
			ti[task].removeDataObject (dynamic_memory_inptr);
	}

	/* Close any memory-related call? */
	DataObject_dynamic *d = ti[task].getTmpDataObject ();
	if (d != NULL)
		for (const auto & event : e.events)
			if (event.Type == EXTRAE_DYNAMIC_MEMORY_OUT_PTR)
			{
				d->setStartAddress (event.Value);
				ti[task].setTmpDataObject (NULL);
				ti[task].addDataObject (d);
				break;
			}

	for (const auto & event : e.events)
	{
		if (thi[thread].getCurrentRegion() > 0)
		{
			if (event.Type >= PAPI_MIN_COUNTER && event.Type <= PAPI_MAX_COUNTER)
			{
				bool found;
				unsigned index = LookupCounter (event.Type, found);
				if (common::DEBUG())
				{
					if (!found)
						cout << "Processing counter " << event.Type << " with value " << event.Value << " at timestamp " << e.Timestamp << endl;
					else
						cout << "Processing counter " << CounterIDNames[index] << " with value " << event.Value << " at timestamp " << e.Timestamp << endl;
				}
	
				processCounter (event, CV);
				storeSample = true;
			}
			if (event.Type >= EXTRAE_SAMPLE_CALLER_MIN && event.Type <= EXTRAE_SAMPLE_CALLER_MAX)
			{
				if (common::DEBUG())
					cout << "Processing C " << event.Type << " at timestamp " << e.Timestamp << endl;

				storeSample = processCaller (event, EXTRAE_SAMPLE_CALLER_MIN, Caller) || storeSample;
			}
			if (event.Type >= EXTRAE_SAMPLE_CALLERLINE_MIN && event.Type <= EXTRAE_SAMPLE_CALLERLINE_MAX)
			{
				if (common::DEBUG())
					cout << "Processing CL " << event.Type << " at timestamp " << e.Timestamp << endl;

				storeSample = processCaller (event, EXTRAE_SAMPLE_CALLERLINE_MIN, CallerLine) || storeSample;
			}
			if (event.Type >= EXTRAE_SAMPLE_CALLERLINE_AST_MIN && event.Type <= EXTRAE_SAMPLE_CALLERLINE_AST_MAX)
			{
				if (common::DEBUG())
					cout << "Processing CL-AST " << event.Type << " at timestamp " << e.Timestamp << endl;

				storeSample = processCaller (event, EXTRAE_SAMPLE_CALLERLINE_AST_MIN, CallerLineAST) || storeSample;
			}
			if (event.Type == EXTRAE_SAMPLE_ADDRESS_LD || 
			    event.Type == EXTRAE_SAMPLE_ADDRESS_ST ||
			    event.Type == EXTRAE_SAMPLE_ADDRESS_MEM_LEVEL || 
			    event.Type == EXTRAE_SAMPLE_ADDRESS_TLB_LEVEL ||
			    event.Type == EXTRAE_SAMPLE_ADDRESS_REFERENCE_CYCLES )
			{
				if (common::DEBUG())
					cout << "Processing sampled address info (" << event.Type << ")" << endl;
				processAddressReference (event, AR);
				storeSample = true;
			}
		}

		if (Semantics == NULL)
		{
			if (event.Type == RegionSeparatorID)
			{
				if (common::DEBUG())
					cout << "Task " << task << " Thread " << thread <<
					  " Found separator (" << RegionSeparatorID <<
					  "," << event.Value << ") at timestamp " << e.Timestamp << endl;

				FoundSeparator = true;
				if (event.Value > 0)
				{
					hasValueSeparator_Start = true;
					ValueSeparator = event.Value;
				}
				else if (event.Value == 0)
					hasValueSeparator_End = true;
			}
		}
	}

	if (Semantics != NULL)
	{
		vector<PRVSemanticValue *> vs = Semantics->getSemantics (e.ObjectID.ptask, e.ObjectID.task, e.ObjectID.thread);
		for (const auto semval : vs)
		{
			if (semval->getFrom() == e.Timestamp + TimeOffset)
			{
				FoundSeparator = true;
				string tmp = semval->getValue().substr (0, semval->getValue().find("."));
				ValueSeparator = atoi (tmp.c_str());
				if (semval->getValue() != "End")
				{
					ValueSeparator = 0;
					for (unsigned i = 0; i < SemanticIndex.size(); i++)
						if (SemanticIndex[i] == semval->getValue())
						{
							ValueSeparator = i+1;
							break;
						}
					if (ValueSeparator == 0)
					{
						SemanticIndex.push_back (semval->getValue());
						ValueSeparator = SemanticIndex.size();
					}
				}
				else
					ValueSeparator = 0;

				if (common::DEBUG())
					cout << "Found semantic separator value " << semval->getValue()
					  << " [" << ValueSeparator << "] at timestamp " << e.Timestamp
					  << endl;
				break;
			}
		}
	}

	if (common::DEBUG())
		cout << "storeSample = " << storeSample << " at timestamp = " << e.Timestamp << endl;

	/* Store this event if a sample has found, but discard starts to regions
	   because accounting starts from 0 on the folded region */
	if (storeSample && 
	     (!hasValueSeparator_Start ||
	     (hasValueSeparator_Start && hasValueSeparator_End)))
	{
		map<unsigned, unsigned long long>::iterator cit;
		map<unsigned, CodeRefTriplet> CodeRefs;
		for (const auto & call : Caller)
		{
			unsigned d = call.first;

			assert (Caller.count(d) == 1);
			assert (CallerLine.count(d) == 1);
			assert (CallerLineAST.count(d) == 1);

			CodeRefTriplet t (Caller[d], CallerLine[d],
			  CallerLineAST[d]);
			CodeRefs[d] = t;
		}

		Sample *s = NULL;

		if (AR.isCompleted())
		{
			s = new Sample (e.Timestamp, e.Timestamp - thi[thread].getStartRegion(),
			  CV, CodeRefs, AR.getReferenceType(), AR.getAddress(),
			  AR.getReferenceMemLevel(), AR.getReferenceTLBLevel(),
			  AR.setReferenceCoreCycles());
		}
		else if (AR.hasAddress()) /* does not contain all info but address is present*/
		{
			s = new Sample (e.Timestamp, e.Timestamp - thi[thread].getStartRegion(),
			  CV, CodeRefs, AR.getReferenceType(), AR.getAddress());
		}
		else
		{
			s = new Sample (e.Timestamp, e.Timestamp - thi[thread].getStartRegion(),
			  CV, CodeRefs);
		}
		assert (s != NULL);

		if (common::DEBUG())
		{
			for (const auto countervalue : CV)
				cout << " " << countervalue.first << " " << countervalue.second;
			cout << endl;
		}
		thi[thread].addSample (s);
	}
	else
	{
		if (common::DEBUG())
			cout << "not adding sample because it is the start of a region!" << endl;
	}

	/* If we found a region separator, increase current region and reset the phase */
	if (hasValueSeparator_Start || hasValueSeparator_End)
	{
		if (hasValueSeparator_End)
		{
			unsigned long long Region = thi[thread].getCurrentRegion();
			if (Region > 0)
			{
				/* Look for the region name */
				string RegionName;
				if (Semantics == NULL)
				{
					bool RegionFound;
					RegionName = getTypeValue (RegionSeparatorID, Region, RegionFound);
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
				  thread, thi[thread].getStartRegion(),
				  e.Timestamp - thi[thread].getStartRegion(),
				  thi[thread].getSamples(),
				  ti[task].getDataObjects(),
				  ti[task].getLivingDataObjects());
			}

			/* Clean */
			thi[thread].clearSamples();
			thi[thread].setSeen (true);
		}

		thi[thread].setCurrentRegion (ValueSeparator);
		thi[thread].setStartRegion (e.Timestamp);
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

	bool has_addresses = false;
	string r = getType (ADDRESS_VARIABLE_ADDRESSES, has_addresses);

	IH.AllocatePTasks (va.size());
	PTaskInformation *pti = IH.getPTasksInformation();
	for (unsigned ptask = 0; ptask < va.size(); ptask++)
	{
		vector<ParaverTraceTask *> vt = va[ptask]->get_tasks();

		if (common::DEBUG())
			cout << " Ptask " << ptask+1 << " contains " << vt.size() << " tasks" << endl;

		pti[ptask].AllocateTasks (vt.size());
		TaskInformation *ti = pti->getTasksInformation();

		/* Copy static data objects */
		if (has_addresses)
			for (const auto eventvalue : pcf->getEventValues (ADDRESS_VARIABLE_ADDRESSES))
			{
				unsigned long long eav, sav;

				string value = pcf->getEventValue (ADDRESS_VARIABLE_ADDRESSES, eventvalue);
	
				/* Parsing string like: a [0x13730100-0x1cfc68ff] */
				string name =
				  value.substr (0, value.find ("[") - 1);
				stringstream sa (
				  value.substr (value.find ("[") + 1, value.find("-") - value.find("[") - 1));
				sa << std::hex;
				sa >> sav;
				stringstream ea (
				  value.substr (value.find ("-") + 1, value.find("]") - value.find ("-") - 1));
				ea << std::hex;
				ea >> eav;
	
				ti->addDataObject (new DataObject_static (sav, eav, name));
			}

		for (unsigned int task = 0; task < vt.size(); task++)
		{
			unsigned nthreads = vt[task]->get_threads().size(); /* o vt[task]->get_threads()[0]->get_key() ? */

			if (common::DEBUG())
				cout << "  Task " << task+1 << " contains " << nthreads << " threads" << endl;

			ti[task].AllocateThreads (nthreads);
		}		
	}

}

string Process::getType (unsigned type, bool &found, bool silent)
{
	found = true;
	string s;

	try
	{ s = pcf->getEventType (type); }
	catch (...)
	{
		if (!silent)
			cerr << "Warning! Did not find the description of event type " << type << " in the PCF file... Will add the definition" << endl; 
		found = false;
		s = "";
	}

	return s;
}

string Process::getTypeValue (unsigned type, unsigned value, bool &found)
{
	found = true;
	string s, cl, c;

	try
	{
	  s = pcf->getEventValue (type, value);
	  cl = pcf->getEventValue (EXTRAE_SAMPLE_CALLERLINE, value);
	  c = pcf->getEventValue (EXTRAE_SAMPLE_CALLER, value);
	}
	catch (...)
	{
		found = false;
		s = "";
	}

	/* If the given type is the same as the caller or caller-line, be careful
	   with special type description compression) */
	if ((s == cl || s == c) && s != "")
	{
		if (s == cl)
		{
			/* Caller Line */
			unsigned line;
			string file;
			stringstream ss;
			pcfcommon::lookForCallerLineInfo (pcf, value, file, line);
			ss << line << "_" << file;
			s = ss.str();
		}
		else
		{
			/* Caller */
			pcfcommon::lookForCallerInfo (pcf, value, s);
		}
	}

	return s;
}

bool Process::anySeenObjects (void) const
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
					return true;
		} 
	}
	return false;
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
		for (const auto & counter : IH.getCounters())
			f << counter << endl;
	f.close();
}

void Process::dumpSeenRegions (string filename)
{
	ofstream f (filename.c_str());
	if (f.is_open())
		for (const auto & region : IH.getRegions())
			f << region << endl;
	f.close();
}

void Process::dumpSeenAddressRegions (string filename)
{
	ofstream f (filename.c_str());
	if (f.is_open())
	{
		PTaskInformation *ptaskinfo = IH.getPTasksInformation();
		for (int ptask = 0; ptask < 1 /* IH.getNumPTasks() */; ptask++)
		{
			TaskInformation *taskinfo = ptaskinfo[ptask].getTasksInformation();
			for (int task = 0; task < 1 /* ptaskinfo[ptask].getNumTasks() */; task++)
			{
				vector<DataObject*> dataobjects =
				  taskinfo[task].getDataObjects ();

				f << std::hex;
				for (auto & d : dataobjects)
					if (dynamic_cast<DataObject_static*>(d) != NULL)
						f << "S " << d->getName() << " 0x" << d->getStartAddress() << " 0x" << d->getEndAddress() << endl;
					else
						f << "D " << d->getName() << " 0x" << d->getStartAddress() << " 0x" << d->getEndAddress() << endl;
						
				f << std::dec;
			} 
		}

	}
	f.close();
}

void Process::dumpSeen (string fnameprefix)
{
	dumpSeenObjects (fnameprefix+".objects");
	dumpSeenCounters (fnameprefix+".counters");
	dumpSeenRegions (fnameprefix+".regions");
	dumpSeenAddressRegions (fnameprefix+".dataobjects");
}

void Process::dumpMissingValuesIntoPCF (void)
{
	const set<unsigned> & missingRegions = IH.getMissingRegions();

	if (missingRegions.size() > 0)
	{
		fstream f (pcffile.c_str(), fstream::out | fstream::app);
		if (f.is_open())
		{
			f << "EVENT_TYPE" << endl
			  << "0 " << RegionSeparatorID << " " << RegionSeparatorName << endl
			  << "VALUES" << endl;

			for (const auto & missingRegion : missingRegions)
			{
				cout << "Warning! Adding missing region label for type " << RegionSeparatorID
				  << " value " << missingRegion << " into " << pcffile << endl;
				stringstream ss;
				ss << missingRegion;
				f << missingRegion << " Value_" << ss.str() << endl;
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
		     << "-semantic F" << endl
             << "-malloc-level L" << endl
		     << "-max-callstack-depth D" << endl;
		exit (-1);
	}

	for (int i = 1; i < argc-1; i++)
	{
		string parameter = argv[i];

		if (parameter == "-separator")
		{
			i++;
			RegionSeparator = argv[i];
			continue;
		}
		else if (parameter == "-max-callstack-depth")
		{
			i++;
			has_max_callstack_depth = true;
			max_callstack_depth = atoi (argv[i]);
			continue;
		}
		else if (parameter == "-malloc-level")
		{
			i++;
			malloc_level = atoi (argv[i]);
			if (malloc_level == 0)
				cerr << "The malloc-level parameter '" << argv[i] << "' was not parsed into a numerical value!" << endl;
			else
				has_malloc_level = malloc_level > 0;
			continue;
		}
		else if (parameter == "-semantic")
		{
			i++;
			if (common::existsFile(argv[i]))
			{
				cout << "Reading semantic file " << argv[i] << endl;
				PRVSemanticCSVName = parameter;
			}
			else
			{
				cerr << "The file " << argv[i] << " does not exist. Dying ... " << endl;
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

	if (RegionSeparator.length() == 0 && PRVSemanticCSVName.length() == 0)
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
	int n;
	if ( ( n = atoi(RegionSeparator.c_str()) ) > 0)
	{
		/* If we have been given an id, prepare it */
		RegionSeparatorID = n;
	}
	else
	{
		/* If we have been given a label, look for it in the PCF */
		RegionSeparatorID = p->lookupType (RegionSeparator);
		if (RegionSeparatorID == 0)
		{
			cerr << "Error! Type with name '" << RegionSeparator << "' was not found in the tracefile PCF" << endl;
			exit (-1);
		}
	}

	bool found;
	cout << "Checking for basic caller information" << flush;
	string t1 = p->getType (EXTRAE_SAMPLE_CALLER_MIN, found);
	if (!found)
	{
		cerr << endl << "Error! Unable to get caller information (event type " << EXTRAE_SAMPLE_CALLER_MIN << ")" << endl;
		exit (-1);
	}
	cout << ", caller line information" << flush;
	string t2 = p->getType (EXTRAE_SAMPLE_CALLERLINE_MIN, found);
	if (!found)
	{
		cerr << endl << "Error! Unable to get caller line information (event type " << EXTRAE_SAMPLE_CALLERLINE_MIN << ")" << endl;
		exit (-1);
	}
	cout << ", caller line AST information" << flush;
	string t3 = p->getType (EXTRAE_SAMPLE_CALLERLINE_AST_MIN, found);
	cout << " Done" << endl;
	if (!found)
	{
		cerr << endl << "Error! Unable to get caller line AST information (event type " << EXTRAE_SAMPLE_CALLERLINE_AST_MIN << ")" << endl;
		exit (-1);
	}

	/* By loading these t1-3 we ensure that the all callers triplets exist */
	if (!p->givenSemanticsCSV())
	{
		bool RegionSeparatorFound;
		RegionSeparatorName = p->getType (RegionSeparatorID, RegionSeparatorFound, false);
		if (!RegionSeparatorFound)
		{
			stringstream ss;
			ss << RegionSeparatorID;
			RegionSeparatorName = "EventType_" + ss.str();
		}
		RegionSeparatorName = common::removeSpaces (RegionSeparatorName);

		cout << "Extracting data for type " << RegionSeparatorID << " (named as " << RegionSeparatorName << ")" << endl;
	}
	else
		cout << "Extracting data for semantic values" << endl;

	p->allocateBuffers ();
	p->parseBody();
	p->closeFile();

	if (!p->anySeenObjects())
	{
		cout << "Error! Unable to extract any information from the tracefile." << endl
		     << "Check that event type exists within the tracefile" << endl;
		exit (-1);
	}

	p->dumpSeen (common::basename (tracename.substr (0, tracename.length()-4)));

	// Emit seen event types when passing a separator manually
	if (!p->givenSemanticsCSV())
		p->dumpMissingValuesIntoPCF ();

	if (p->getNCounterChanges() > 0)
		cout << "Ignored " << p->getNCounterChanges() << " instances, most probably because of hardware counter set change." << endl;

	string ControlFile = common::basename (tracename.substr (0, tracename.length()-4) + ".control");
	ofstream cfile (ControlFile.c_str());
	cfile << tracename << endl;
	cfile << RegionSeparatorID << endl;
	cfile << RegionSeparatorName << endl;
	cfile.close ();

	return 0;
}

