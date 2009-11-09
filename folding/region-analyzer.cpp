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

static char UNUSED rcsid[] = "$Id$";


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
#include "region-analyzer.h"

#define UNREFERENCED(a)    ((a) = (a))
#define PAPI_MIN_COUNTER   42000000
#define PAPI_MAX_COUNTER   42009998

using namespace std;

namespace libparaver {

class Process : public ParaverTrace
{
	private:
	Region *currentRegion;
	string *CounterIDs;

	unsigned long long TimeLimit_Type;
	unsigned long long TimeLimit_Value;
	unsigned long long TimeLimit_Start;
	unsigned long long TimeLimit_End;
	unsigned long long Separator;
	unsigned task;
	unsigned thread;
	bool useTimeLimit;
	bool TimeRegionLimit_entered;
	bool TimeRegionLimit_exited;
	unsigned numCounterIDs;
	
	bool LookupCounter (string Counter, unsigned *index);

	public:
	list<Region*> foundRegions;
	unsigned long long TimeLimit_out_Start;
	unsigned long long TimeLimit_out_End;
	void setTimeLimit (unsigned long long Tstart, unsigned long long Tend);
	void setTimeRegionLimit (unsigned long long Type, unsigned long long Value);

	Process (string prvFile, bool multievents, int task, int thread, vector<string> &CounterList, unsigned long long Separator);

	void processState (struct state_t &s);
	void processMultiEvent (struct multievent_t &e);
	void processEvent (struct singleevent_t &e);
	void processCommunication (struct comm_t &c);
	void processCommunicator (string &c);
	void processComment (string &c);
};

Process::Process (string prvFile, bool multievents, int task, int thread, vector<string> &CounterList, unsigned long long Separator) : ParaverTrace (prvFile, multievents)
{
	numCounterIDs = CounterList.size();
	CounterIDs = new string[numCounterIDs];

	int j = 0;
	for (vector<string>::iterator i = CounterList.begin(); i != CounterList.end(); j++, i++)
		CounterIDs[j] = *i;

	currentRegion = NULL;
	this->Separator = Separator;
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

bool Process::LookupCounter (string Counter, unsigned *index)
{
	for (unsigned i = 0; i < numCounterIDs; i++)
		if (CounterIDs[i] == Counter)
		{
			*index = i;
			return true;
		}

	return false;
}

void Process::processMultiEvent (struct multievent_t &e)
{
	bool FoundSeparator = false;
	unsigned long long ValueSeparator = 0;
	unsigned long long TypeSeparator = 0;

	if (e.ObjectID.task-1 != task || e.ObjectID.thread-1 != thread)
		return;

	/* If using time limit politics, check if we are within limits */
	if (useTimeLimit)
	{
		TimeRegionLimit_entered = e.Timestamp >= TimeLimit_Start;
		TimeRegionLimit_exited = e.Timestamp >= TimeLimit_End;
	}

	/* Check if we are entering the time region if we weren't inside */
	if (!useTimeLimit && !TimeRegionLimit_entered && !TimeRegionLimit_exited)
	{
		for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
			if ((*it).Type == TimeLimit_Type && (*it).Value == TimeLimit_Value)
			{
				TimeLimit_out_Start = e.Timestamp;
				TimeRegionLimit_entered = true;
			}
		/* If we haven't entered the time region, return! */
		if (!TimeRegionLimit_entered)
			return;
	}

	if (TimeRegionLimit_entered && !TimeRegionLimit_exited)
	{
		for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
			if ((*it).Type == Separator)
			{
				FoundSeparator = true;
				TypeSeparator = (*it).Type;
				ValueSeparator = (*it).Value;
				break;
			}

		if (FoundSeparator && ValueSeparator != 0)
		{
			/* Start a region */
			currentRegion = new Region (e.Timestamp, TypeSeparator, ValueSeparator);
			for (unsigned i = 0; i < numCounterIDs; i++)
				currentRegion->HWCvalues.push_back (0);
		}
		else
		{
			/* Ignore counters from initial region */
			if (currentRegion != NULL)
			{
				for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
				{
					if ((*it).Type >= PAPI_MIN_COUNTER && (*it).Type <= PAPI_MAX_COUNTER)
					{
						unsigned index;
						stringstream tmp;
						tmp << (*it).Type;
						if (LookupCounter (tmp.str(), &index))
							currentRegion->HWCvalues[index] += (*it).Value;
					}
				}
			}
		}

		if (FoundSeparator && ValueSeparator == 0 && currentRegion != NULL)
		{
			/* Complete a region */
			currentRegion->Tend = e.Timestamp;
			foundRegions.push_back (currentRegion);
			currentRegion = NULL;
		}
	}

	/* Check if we are leaving the time region if we were inside */
	if (!useTimeLimit && TimeRegionLimit_entered && !TimeRegionLimit_exited)
	{
		for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
			if ((*it).Type == TimeLimit_Type && (*it).Value == 0)
			{
				TimeLimit_out_End = e.Timestamp;
				TimeRegionLimit_exited = true;
			}
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

void Process::setTimeLimit (unsigned long long Tstart, unsigned long long Tend)
{
	useTimeLimit = true;
	TimeLimit_Start = Tstart;
	TimeLimit_End = Tend;
	TimeRegionLimit_entered = TimeRegionLimit_exited = false;
}

void Process::setTimeRegionLimit (unsigned long long Type, unsigned long long Value)
{
	useTimeLimit = false;
	TimeLimit_Type = Type;
	TimeLimit_Value = Value;
	TimeRegionLimit_entered = TimeRegionLimit_exited = false;
}

} /* namespace libparaver */

using namespace::libparaver;
using namespace::std;

void SearchForRegionsWithinRegion (string tracename, unsigned task, unsigned thread,
	unsigned long long Type, unsigned long long TimeType, unsigned long long TimeValue,
	unsigned long long *out_Tstart, unsigned long long *out_Tend,
	vector<string> &lCounters, list<Region*> &foundRegions)
{
	Process *p = new Process (tracename, true, task, thread, lCounters, Type);
	p->setTimeRegionLimit (TimeType, TimeValue);

	p->parseBody();

	*out_Tstart = p->TimeLimit_out_Start;
	*out_Tend = p->TimeLimit_out_End;
	foundRegions = p->foundRegions;

#if defined(DEBUG)
	for (list<Region *>::iterator i = p->foundRegions.begin();
	  i != p->foundRegions.end(); i++)
	{
		cout << "Found region " << (*i)->Type << ":" << (*i)->Value << " from " << (*i)->Tstart << " to " << (*i)->Tend << endl;
		cout << "COUNTERS:";
		vector<string>::iterator n = lCounters.begin();
		for (vector<unsigned long long>::iterator j = (*i)->HWCvalues.begin();
		  j != (*i)->HWCvalues.end(); j++, n++)
			cout << " "<< *n << "=>" << *j;
		cout << endl;
	}
#endif
}

void SearchForRegionsWithinTime (string tracename, unsigned task, unsigned thread,
	unsigned long long Type, unsigned long long Tstart, unsigned long long Tend,
	unsigned long long *out_Tstart, unsigned long long *out_Tend,
	vector<string> &lCounters, list<Region*> &foundRegions)
{
	Process *p = new Process (tracename, true, task, thread, lCounters, Type);
	p->setTimeLimit (Tstart, Tend);

	p->parseBody();

	*out_Tstart = Tstart;
	*out_Tend = Tend;
	foundRegions = p->foundRegions;

#if defined(DEBUG)
	for (list<Region *>::iterator i = p->foundRegions.begin();
	  i != p->foundRegions.end(); i++)
	{
		cout << "Found region " << (*i)->Type << ":" << (*i)->Value << " from " << (*i)->Tstart << " to " << (*i)->Tend << endl;
		cout << "COUNTERS:";
		vector<string>::iterator n = lCounters.begin();
		for (vector<unsigned long long>::iterator j = (*i)->HWCvalues.begin();
		  j != (*i)->HWCvalues.end(); j++, n++)
			cout << " "<< *n << "=>" << *j;
		cout << endl;
	}
#endif
}

