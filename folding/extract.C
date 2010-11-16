/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  MPItrace                                 *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *                                                             ___           *
 *   +---------+     http:// www.cepba.upc.edu/tools_i.htm    /  __          *
 *   |    o//o |     http:// www.bsc.es                      /  /  _____     *
 *   |   o//o  |                                            /  /  /     \    *
 *   |  o//o   |     E-mail: cepbatools@cepba.upc.edu      (  (  ( B S C )   *
 *   | o//o    |     Phone:          +34-93-401 71 78       \  \  \_____/    *
 *   +---------+     Fax:            +34-93-401 25 77        \  \__          *
 *    C E P B A                                               \___           *
 *                                                                           *
 * This software is subject to the terms of the CEPBA/BSC license agreement. *
 *      You must accept the terms of this license to use this software.      *
 *                                 ---------                                 *
 *                European Center for Parallelism of Barcelona               *
 *                      Barcelona Supercomputing Center                      *
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

#include "ParaverTrace.h"
#include "ParaverTraceThread.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceApplication.h"
#include "UIParaverTraceConfig.h"
#include "common.H"

#include <sstream>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <list>

#define PAPI_MIN_COUNTER   42000000
#define PAPI_MAX_COUNTER   42009998

unsigned long long RegionSeparator = 123456;
vector<unsigned long long> SkipTypes;
vector<unsigned long long> PhaseSeparators;

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

class LineCodeInformation
{
	public:
	unsigned line;
	unsigned lineid_type;
	unsigned lineid_value;
	unsigned long long time;
};

class ThreadInformation
{
	public:
	list<unsigned long long> TimeSamples;
	list<bool> SkipSamples;
	list<unsigned long long> * CounterSamples;
	list<LineCodeInformation*> LineSamples;
	ofstream output;

	unsigned long long CurrentIteration;
	unsigned long long CurrentPhase;
	unsigned long long CurrentRegion;
	unsigned long long StartRegion;

	void AllocateBufferCounters (int numCounters);
	ThreadInformation ();
};

ThreadInformation::ThreadInformation ()
{
	CurrentRegion = CurrentPhase = CurrentIteration = 0;
}

void ThreadInformation::AllocateBufferCounters (int numCounters)
{
	CounterSamples = new list<unsigned long long> [numCounters];
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

class InformationHolder
{
	private:
	int numTasks;
	TaskInformation *TasksInfo;
	
	public:
	int getNumTasks (void)
	{ return numTasks; };

	TaskInformation* getTasksInformation (void)
	{ return TasksInfo; };

	void AllocateTasks (int numTasks);
	~InformationHolder();
};

InformationHolder::~InformationHolder()
{
	delete [] TasksInfo;
}

void InformationHolder::AllocateTasks (int numTasks)
{
	this->numTasks = numTasks;
	TasksInfo = new TaskInformation[this->numTasks];
}

class Process : public ParaverTrace
{
	private:
	list<unsigned> CallerCut;
	string *CounterIDNames;
	unsigned long long *CounterIDs;
	unsigned long long LookupCounter (unsigned long long Counter);
	UIParaverTraceConfig *pcf;
	void ReadCallerLinesIntoList (string file, UIParaverTraceConfig *pcf);

	public:
	Process (string prvFile, bool multievents);

	void processState (struct state_t &s);
	void processMultiEvent (struct multievent_t &e);
	void processEvent (struct singleevent_t &e);
	void processCommunication (struct comm_t &c);
	void processCommunicator (string &c);
	void processComment (string &c);
	InformationHolder IH;
	unsigned numCounterIDs;
	list<string> TypeValuesLabels;
};

Process::Process (string prvFile, bool multievents) : ParaverTrace (prvFile, multievents)
{
	unsigned found_counters = 0;

	/* Look for hw counters */
	string pcffile = prvFile.substr (0, prvFile.rfind(".prv")) + string (".pcf");

	pcf = new UIParaverTraceConfig (pcffile);

	for (int i = PAPI_MIN_COUNTER; i <= PAPI_MAX_COUNTER; i++)
	{
		string s = pcf->getEventType (i);
		if (s.length() > 0)
			found_counters++;
	}

	vector<unsigned> values = pcf->getEventValuesFromEventTypeKey(RegionSeparator);
	for (unsigned i = 0; i < values.size(); i++)
		TypeValuesLabels.push_back (pcf->getEventValue(RegionSeparator, values[i]));

	numCounterIDs = found_counters;
	CounterIDs = new unsigned long long[numCounterIDs];
	CounterIDNames = new string[numCounterIDs];

	unsigned j = 0;
	for (unsigned i = PAPI_MIN_COUNTER; i <= PAPI_MAX_COUNTER; i++)
	{
		string s = pcf->getEventType (i);
		if (s.length() > 0)
		{
			CounterIDs[j] = i;
			CounterIDNames[j] = s.substr (s.find ('(')+1, s.rfind (')') - (s.find ('(') + 1));
			j++;
		}
	}

	ReadCallerLinesIntoList ("list", pcf);
}

void Process::ReadCallerLinesIntoList (string file, UIParaverTraceConfig *pcf)
{
	string str;
	list<string> l_tmp;
	fstream file_op (file.c_str(), ios::in);


	if (file_op.is_open())
	{
		while (file_op >> str)
		{
			/* Add element into list if it didn't exist */
			list<string>::iterator iter = find (l_tmp.begin(), l_tmp.end(), str);
			if (iter == l_tmp.end())
				l_tmp.push_back (str);
		}
		file_op.close();
	
		vector<unsigned> v = pcf->getEventValuesFromEventTypeKey (30000000);
		unsigned i = 2;
		while (i != v.size())
		{
			string str = pcf->getEventValue (30000000, v[i]);
			string func_name = str.substr (0, str.find(' '));
			if (find (l_tmp.begin(), l_tmp.end(), func_name) != l_tmp.end())
			{
				cout << "Adding identifier " << i << " for caller " << pcf->getEventValue (30000000, i) << endl;
				CallerCut.push_back (i);
			}
			i++;
		}
	}
	else
	{
		cout << "WARNING: No callstack segment cut given!" << endl;
		return;
	}

}

unsigned long long Process::LookupCounter (unsigned long long Counter)
{
	for (unsigned i = 0; i < numCounterIDs; i++)
		if (CounterIDs[i] == Counter)
			return i;

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

void Process::processMultiEvent (struct multievent_t &e)
{
	bool FoundSeparator = false;
	bool FoundPhaseSeparator = false;
	unsigned long long ValueSeparator = 0;
	int task = e.ObjectID.task - 1;
	int thread = e.ObjectID.thread - 1;

	if (task >= IH.getNumTasks())
		return;

	TaskInformation *ti = IH.getTasksInformation();
	if (thread >= ti[task].getNumThreads())
		return;

  ThreadInformation *thi = &((ti[task].getThreadsInformation())[thread]);

	for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
		if ((*it).Type == 123456)
			thi->CurrentIteration = (*it).Value;

	for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
	{
		if ((*it).Type == RegionSeparator)
		{
			FoundSeparator = true;
			ValueSeparator = (*it).Value;
			break;
		}
		FoundPhaseSeparator = FoundPhaseSeparator || find (PhaseSeparators.begin(), PhaseSeparators.end(), (*it).Type) != PhaseSeparators.end();
	}

	/* If we haven't found a separator, or it's and end separator, add counters to
	   the existing working set */
	if ((!FoundSeparator || (FoundSeparator && ValueSeparator == 0)) && thi->CurrentRegion != 0)
	{
		/* First check if this record should be skipped at dumping the counters! */
		bool skip = false;
		for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
			skip = (find (SkipTypes.begin(), SkipTypes.end(), (*it).Type ) != SkipTypes.end()) || skip;

#if defined(DEBUG)
		if (!skip)
		{
			cout << "will not skip @ " << e.Timestamp << " : ";
			for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
				cout << (*it).Type << " ";
			cout << endl;
		}
#endif

		bool CounterAdded = false;
		for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
		{
			if ((*it).Type >= PAPI_MIN_COUNTER && (*it).Type <= PAPI_MAX_COUNTER)
			{
				int index = LookupCounter ((*it).Type);
				thi->CounterSamples[index].push_back ((*it).Value);
				CounterAdded = true;
			}
		}
		if (CounterAdded)
		{
			thi->TimeSamples.push_back (e.Timestamp);
			thi->SkipSamples.push_back (skip);
		}

		bool found_min_level = false;
		unsigned MinCallerLevel = 0, MinCallerLevelValue = 0;

		for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
		{
			if ((*it).Type >= 30000000 && (*it).Type <= 30000099)
			{
				if (find (CallerCut.begin(), CallerCut.end(), (*it).Value) != CallerCut.end())
				{
					if (!found_min_level)
					{
						found_min_level = true;
						MinCallerLevel = (*it).Type;
					}
					else if (found_min_level && MinCallerLevel > (*it).Type)
					{
						MinCallerLevel = (*it).Type;
					}
				}
			}
		}

		if (found_min_level)
		{
			bool found = false;
			for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
			{
				if (MinCallerLevel + 100 == (*it).Type)
				{
					MinCallerLevel = (*it).Type;
					MinCallerLevelValue = (*it).Value;
					found = true;
				}
			}
			if (!found)
			{
				cerr << "Couldn't find callerline pair for caller " << MinCallerLevel << ":" << MinCallerLevelValue << " at timestamp " << e.Timestamp << endl;
				exit (-1);
			}
		}

		if (found_min_level)
		{
			string CallerLine = pcf->getEventValue (MinCallerLevel, MinCallerLevelValue);
			if (CallerLine.length() > 0 && CallerLine != "Not found")
			{
				LineCodeInformation *lci = new LineCodeInformation;
				lci->line = atoi ((CallerLine.substr (0, CallerLine.find (" "))).c_str());
				lci->lineid_value = MinCallerLevelValue;
				lci->lineid_type = MinCallerLevel;
				lci->time = e.Timestamp;
				thi->LineSamples.push_back (lci);
			}
		}
	}

	/* If we found an end of a region or a phase change inside a region, dump normalized samples */
	if ((FoundSeparator && ValueSeparator == 0) || FoundPhaseSeparator)
	{
		unsigned long long TotalTime = e.Timestamp - thi->StartRegion;

		/* First dump HW counters */
		if (thi->TimeSamples.size() <= 1)
		{
			thi->TimeSamples.clear();
			thi->SkipSamples.clear();
			for (unsigned cnt = 0; cnt < numCounterIDs; cnt++)
				thi->CounterSamples[cnt].clear();
		}
		else
		{
			if (thi->TimeSamples.size() > 0)
			{
				string RegionNameValue = pcf->getEventValue (RegionSeparator, thi->CurrentRegion);

				/* Write the total time spent in this region */
				if (RegionNameValue.length() > 0 && RegionNameValue != "Not found")
				{
					thi->output << "T " << common::removeSpaces (RegionNameValue) << "." << thi->CurrentPhase << " " << TotalTime << endl;
				}
				else
				{
					string RegionName = pcf->getEventType (RegionSeparator);
					if (RegionName.length() > 0)
						thi->output << "T " << common::removeSpaces (RegionName) << "_" << thi->CurrentRegion << "." << thi->CurrentPhase << " " << TotalTime << endl;
					else
						thi->output << "T Unknown_" << thi->CurrentRegion << "." << thi->CurrentPhase << " " << TotalTime << endl;
				}

				/* Write total counters spent in this region */
				for (unsigned cnt = 0; cnt < numCounterIDs; cnt++)
				{
					unsigned long long TotalCounter = 0;
					list<unsigned long long>::iterator Counter_iter = thi->CounterSamples[cnt].begin();
					for ( ; Counter_iter != thi->CounterSamples[cnt].end(); Counter_iter++)
						TotalCounter += (*Counter_iter);
					if (TotalCounter > 0)
						thi->output << "A " << CounterIDNames[cnt] << " " << TotalCounter << endl;
				}
			}

			for (unsigned cnt = 0; cnt < numCounterIDs; cnt++)
			{
				unsigned long long TotalCounter = 0;
				list<unsigned long long>::iterator Counter_iter = thi->CounterSamples[cnt].begin();
				for ( ; Counter_iter != thi->CounterSamples[cnt].end(); Counter_iter++)
					TotalCounter += (*Counter_iter);

				list<unsigned long long>::iterator Times_iter = thi->TimeSamples.begin();
				Counter_iter = thi->CounterSamples[cnt].begin();
				list<unsigned long long>::iterator Counter_iter_next = thi->CounterSamples[cnt].begin();
				Counter_iter_next++;
				list<bool>::iterator Skip_iter = thi->SkipSamples.begin();

#if defined(DEBUG)
				thi->output << "TOTAL TIME = " << TotalTime << " from " << thi->StartRegion << " to " << e.Timestamp << " TOTAL COUNTER[" << CounterIDs[cnt] << "/"<< cnt << "]= " << TotalCounter << endl;
#endif

				if (TotalCounter > 0)
				{
					unsigned long long AccumCounter = 0;

					/* Last pair is always 1 - 1, skip them! */
					for (; Counter_iter_next != thi->CounterSamples[cnt].end();
						Counter_iter_next++, Counter_iter++, Times_iter++, Skip_iter++)
					{
						if (!(*Skip_iter))
						{
							double NTime = ::NormalizeValue ((*Times_iter) - thi->StartRegion, 0, TotalTime);
							double NCounter = ::NormalizeValue (AccumCounter + (*Counter_iter), 0, TotalCounter);
	
#if defined(DEBUG)
							thi->output << "S " << CounterIDNames[cnt] << " / " <<  CounterIDs[cnt] << " " << NTime << " " << NCounter << " (TIME = " << (*Times_iter) << ", TOTALTIME= " << (*Times_iter) - thi->StartRegion<< "  )" << endl;
#else
							thi->output << "S " << CounterIDNames[cnt] << " " << NTime << " " << NCounter << " " << (*Times_iter) - thi->StartRegion << " " << (AccumCounter + (*Counter_iter)) << " " << thi->CurrentIteration << endl;
#endif
						}

						/* Always! accumulate counters, even for skipped records! */
						AccumCounter += (*Counter_iter);
					}
					thi->CounterSamples[cnt].clear();
				} /* TotalCounter > 0 */
			} /* !Skip */
			thi->TimeSamples.clear();
			thi->SkipSamples.clear();
		}

		/* Then dump caller line information */
		for (list<LineCodeInformation*>::iterator it = thi->LineSamples.begin(); it != thi->LineSamples.end(); it++)
		{
			double NTime = ::NormalizeValue ((*it)->time - thi->StartRegion, 0, TotalTime);

#if defined(DEBUG)
			thi->output << "S LINE " << NTime << " " << (*it)->line << " at timestamp " << (*it)->time << endl;
			thi->output << "S LINEID " << NTime << " " << (*it)->lineid_value << " at timestamp " << (*it)->time << endl;
#else
			thi->output << "S LINE " << NTime << " " << (*it)->line << " " << (*it)->time << " 0" << endl;
			thi->output << "S LINEID " << NTime << " " << (*it)->lineid_value << " " << (*it)->time << " 0" << endl;
#endif
		}
		thi->LineSamples.clear();

	}

	/* If we found a phase separator, increase current phase */
	if (FoundPhaseSeparator)
		thi->CurrentPhase++;

	/* If we found a region separator, increase current region and reset the phase */
	if (FoundSeparator)
	{
		thi->CurrentRegion = ValueSeparator;
		thi->CurrentPhase = 0;
	}

	if ((FoundSeparator && ValueSeparator != 0) || FoundPhaseSeparator)
		thi->StartRegion = e.Timestamp;
}

void Process::processEvent (struct singleevent_t &e)
{
	UNREFERENCED(e);
}

void Process::processCommunication (struct comm_t &c)
{
	UNREFERENCED(c);
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
		     << "-phase-separator S" << endl
		     << "-skip-type T" <<  endl;
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
		else if (strcmp ("-skip-type", argv[i]) == 0)
		{
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid number of type in '-skip-type' option" << endl;
			else
				SkipTypes.push_back (atoi(argv[i]));
		}
		else if (strcmp ("-phase-separator", argv[i]) == 0)
		{
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid number of type in '-phase-separator' option" << endl;
			else
				PhaseSeparators.push_back (atoi(argv[i]));
		}
	}

	return argc-1;
}



int main (int argc, char *argv[])
{
	int res = ProcessParameters (argc, argv);

	string tracename = string(argv[res]);

	Process *p = new Process (tracename, true);

	vector<ParaverTraceApplication *> va = p->get_applications();
	if (va.size() != 1)
	{
		cerr << "ERROR Cannot parse traces with more than one application" << endl;
		return -1;
	}

	if (p->numCounterIDs == 0)
	{
		cerr << "ERROR! Cannot find performance counters in the PCF file" << endl;
		exit (-1);
	}

	if (p->TypeValuesLabels.size() == 0)
	{
		cerr << "ERROR! Cannot find regions delimited by type " << RegionSeparator << endl;
		exit (-1);
	}

	for (unsigned int i = 0; i < va.size(); i++)
	{
		vector<ParaverTraceTask *> vt = va[i]->get_tasks();
		p->IH.AllocateTasks (vt.size());
		TaskInformation *ti = p->IH.getTasksInformation();
		for (unsigned int j = 0; j < vt.size(); j++)
		{
			ti[j].AllocateThreads (vt[j]->get_threads().size());
			for (unsigned int k = 0; k < vt[j]->get_threads().size(); k++)
			{
				ThreadInformation *thi = &((ti[j].getThreadsInformation())[k]);
				thi->AllocateBufferCounters (p->numCounterIDs);

				stringstream tasknumber, threadnumber;
				tasknumber << j+1;
				threadnumber << k+1;
				string completefile = tracename.substr (0, tracename.length()-4) + ".extract." + tasknumber.str() + "." + threadnumber.str();
				thi->output.open (completefile.c_str());

				/* Put definitions for the separator event */
				list<string>::iterator it = p->TypeValuesLabels.begin();
				for (; it != p->TypeValuesLabels.end(); it++)
					thi->output << "D " << *it << endl;
			}
		}
  }

	p->parseBody();

	for (unsigned int i = 0; i < va.size(); i++)
	{
		vector<ParaverTraceTask *> vt = va[i]->get_tasks();
		TaskInformation *ti = p->IH.getTasksInformation();
		for (unsigned int j = 0; j < vt.size(); j++)
			for (unsigned int k = 0; k < vt[j]->get_threads().size(); k++)
			{
				ThreadInformation *thi = &((ti[j].getThreadsInformation())[k]);
				thi->output.close();
			}
	}

	string ControlFile = tracename.substr (0, tracename.length()-4) + ".control";
	ofstream cfile (ControlFile.c_str());
	cfile << tracename << endl;
	cfile << RegionSeparator << endl;
	cfile << PhaseSeparators.size() << endl;
	vector<unsigned long long>::iterator it = PhaseSeparators.begin();
	while (it != PhaseSeparators.end())
	{
		cfile << (*it) << endl;
		it++;
	}
	cfile.close ();

	return 0;
}

