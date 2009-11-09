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

static char rcsid[] = "$Id$";

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

#define UNREFERENCED(a)    ((a) = (a))
#define PAPI_MIN_COUNTER   42000000
#define PAPI_MAX_COUNTER   42009998

unsigned long long RegionSeparator = 123456;
vector<unsigned long long> SkipTypes;

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
	public:
	list<unsigned long long> TimeSamples;
	list<bool> SkipSamples;
	list<unsigned long long> * CounterSamples;
	ofstream output;

	unsigned long long CurrentRegion;
	unsigned long long StartRegion;

	void AllocateBufferCounters (int numCounters);
	ThreadInformation ();
};

ThreadInformation::ThreadInformation ()
{
	CurrentRegion = 0;
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
	const int getNumThreads (void)
	{ return numThreads; };

	ThreadInformation* getThreadsInformation (void)
	{ return ThreadsInfo; };

	void AllocateThreads (int numThreads);
	~TaskInformation();
};

TaskInformation::~TaskInformation()
{
	for (int i = 0; i < numThreads; i++)
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
	const int getNumTasks (void)
	{ return numTasks; };

	TaskInformation* getTasksInformation (void)
	{ return TasksInfo; };

	void AllocateTasks (int numTasks);
	~InformationHolder();
};

InformationHolder::~InformationHolder()
{
	for (int i = 0; i < numTasks; i++)
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
	ofstream traceout;
	unsigned long long *CounterIDs;
	unsigned numCounterIDs;
	unsigned long long LookupCounter (unsigned long long Counter);

	public:
	Process (string prvFile, bool multievents, list<unsigned long long> CounterList);
	~Process ();

	void processState (struct state_t &s);
	void processMultiEvent (struct multievent_t &e);
	void processEvent (struct singleevent_t &e);
	void processCommunication (struct comm_t &c);
	void processCommunicator (string &c);
	void processComment (string &c);
	InformationHolder IH;
};

Process::Process (string prvFile, bool multievents, list<unsigned long long> CounterList) : ParaverTrace (prvFile, multievents)
{
	numCounterIDs = CounterList.size();
	CounterIDs = new unsigned long long[numCounterIDs];

	int j = 0;
	for (list<unsigned long long>::iterator i = CounterList.begin(); i != CounterList.end(); j++, i++)
		CounterIDs[j] = *i;
}

unsigned long long Process::LookupCounter (unsigned long long Counter)
{
	for (unsigned i = 0; i < numCounterIDs; i++)
		if (CounterIDs[i] == Counter)
			return i;

	return numCounterIDs;
}

Process::~Process ()
{
	traceout.close ();
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
		if ((*it).Type == RegionSeparator)
		{
			FoundSeparator = true;
			ValueSeparator = (*it).Value;
			break;
		}

	/* If we haven't found a separator, or it's and end separator, add counters to
	   the existing working set */
	if ((!FoundSeparator || (FoundSeparator && ValueSeparator == 0)) && thi->CurrentRegion != 0)
	{
		/* First check if this record should be skipped at dumping the counters! */
		bool skip = false;
		for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
			skip = (find (SkipTypes.begin(), SkipTypes.end(), (*it).Type ) != SkipTypes.end()) || skip;

#if 0
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
	}

	/* If we found an end of a region, dump normalized samples */
	if (FoundSeparator && ValueSeparator == 0)
	{
		if (thi->TimeSamples.size() <= 1)
		{
			thi->TimeSamples.clear();
			thi->SkipSamples.clear();
			for (unsigned cnt = 0; cnt < numCounterIDs; cnt++)
				thi->CounterSamples[cnt].clear();
		}
		else
		{
			unsigned long long TotalTime = e.Timestamp - thi->StartRegion;

			if (thi->TimeSamples.size() > 0)
				thi->output << "T " << thi->CurrentRegion << " " << TotalTime << endl;

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
				cout << "TOTAL TIME = " << TotalTime << " from " << thi->StartRegion << " to " << e.Timestamp << " TOTAL COUNTER[" << CounterIDs[cnt] << "/"<< cnt << "]= " << TotalCounter << endl;
#endif

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
						cout << "S " << CounterIDs[cnt] << " " << NTime << " " << NCounter << " (TIME = " << (*Times_iter) << ", TOTALTIME= " << (*Times_iter) - thi->StartRegion<< "  )" << endl;
#else
						thi->output << "S " << CounterIDs[cnt] << " " << NTime << " " << NCounter << endl;
#endif
					}

					/* Always! accumulate counters, even for skipped records! */
					AccumCounter += (*Counter_iter);
				}
				thi->CounterSamples[cnt].clear();
			} /* !Skip */
			thi->TimeSamples.clear();
			thi->SkipSamples.clear();
		}
	}

	if (FoundSeparator)
		thi->CurrentRegion = ValueSeparator;
	if (FoundSeparator && ValueSeparator != 0)
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
    cerr << "Insufficient number of parameters" << endl;
    cerr << "Available options are: " << endl
         << "-separator S" << endl
		     << "-skip-type T" <<  endl;
    exit (-1);
  }

  for (int i = 1; i < argc-1; i++)
  {
    if (strcmp ("-separator",  argv[i]) == 0)
    {
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid number of iteration" << endl;
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
	}

	return argc-1;
}


int main (int argc, char *argv[])
{
	int res = ProcessParameters (argc, argv);

	string tracename = string(argv[res]);

	/* Look for hw counters */
  string pcffile = tracename.substr (0, tracename.length()-3) + string ("pcf");
  UIParaverTraceConfig *pcf = new UIParaverTraceConfig (pcffile);

	list<unsigned long long> CountersList;
	for (int i = PAPI_MIN_COUNTER; i <= PAPI_MAX_COUNTER; i++)
	{
		string s = pcf->getEventType (i);
		if (s.length() > 0)
			CountersList.push_back (i);
	}

	Process *p = new Process (tracename, true,CountersList);

	vector<ParaverTraceApplication *> va = p->get_applications();
	if (va.size() != 1)
	{
		cerr << "ERROR Cannot parse traces with more than one application" << endl;
		return -1;
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
				thi->AllocateBufferCounters (CountersList.size());

				stringstream tasknumber, threadnumber;
				tasknumber << j;
				threadnumber << k;
				string completefile = tracename.substr (0, tracename.length()-4) + ".extract." + tasknumber.str() + "." + threadnumber.str();
				thi->output.open (completefile.c_str());
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

	return 0;
}

