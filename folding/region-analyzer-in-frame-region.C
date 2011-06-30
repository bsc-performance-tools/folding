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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/folding/region-analyzer.C $
 | 
 | @last_commit: $Date: 2009-12-03 11:30:14 +0100 (dj, 03 des 2009) $
 | @version:     $Revision: 66 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id: region-analyzer.C 66 2009-12-03 10:30:14Z harald $";

#include "ParaverTrace.h"
#include "ParaverTraceThread.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceApplication.h"
#include "UIParaverTraceConfig.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <string.h>
#include <list>

#include "region-analyzer-in-frame-region.H"
#include "common.H"

#define PAPI_MIN_COUNTER   42000000
#define PAPI_MAX_COUNTER   42009998

using namespace std;

namespace libparaver {

class ProcessFrameRegion : public ParaverTrace
{
	private:
	Region *currentRegion;
	unsigned *CounterIDs;

	unsigned long long TimeLimit_Type;
	unsigned long long TimeLimit_Value;
	unsigned long long TimeLimit_Start;
	unsigned long long TimeLimit_End;
	unsigned long long Separator;
	unsigned task;
	unsigned thread;
	bool TimeRegionLimit_entered;
	bool TimeRegionLimit_exited;
	unsigned numCounterIDs;
	
	bool LookupCounter (unsigned Counter, unsigned *index);

	public:
	list<Region*> foundRegions;
	unsigned long long TimeLimit_out_Start;
	unsigned long long TimeLimit_out_End;
	void setTimeRegionLimit (unsigned long long Type, unsigned long long Value);

	ProcessFrameRegion (string prvFile, bool multievents, int task, int thread, vector<unsigned> &CounterList, unsigned long long Separator);

	void processState (struct state_t &s);
	void processMultiEvent (struct multievent_t &e);
	void processEvent (struct singleevent_t &e);
	void processCommunication (struct comm_t &c);
	void processCommunicator (string &c);
	void processComment (string &c);
};

ProcessFrameRegion::ProcessFrameRegion (string prvFile, bool multievents, int task, int thread, vector<unsigned> &CounterList, unsigned long long Separator) : ParaverTrace (prvFile, multievents)
{
	numCounterIDs = CounterList.size();
	CounterIDs = new unsigned [numCounterIDs];

	for (unsigned i = 0; i < CounterList.size(); i++)
		CounterIDs[i] = CounterList[i];

	currentRegion = NULL;
	this->Separator = Separator;
	this->task = task;
	this->thread = thread;
}

void ProcessFrameRegion::processComment (string &c)
{
	UNREFERENCED(c);
}

void ProcessFrameRegion::processCommunicator (string &c)
{
	UNREFERENCED(c);
}

void ProcessFrameRegion::processState (struct state_t &s)
{
	UNREFERENCED(s);
}

bool ProcessFrameRegion::LookupCounter (unsigned Counter, unsigned *index)
{
	for (unsigned i = 0; i < numCounterIDs; i++)
		if (CounterIDs[i] == Counter)
		{
			*index = i;
			return true;
		}

	return false;
}

void ProcessFrameRegion::processMultiEvent (struct multievent_t &e)
{
	bool FoundSeparator = false;
	unsigned long long ValueSeparator = 0;
	unsigned long long TypeSeparator = 0;

	if (e.ObjectID.task != task || e.ObjectID.thread != thread)
		return;

	/* Check if we are entering the time region if we weren't inside */
	if (!TimeRegionLimit_entered && !TimeRegionLimit_exited)
	{
		for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
			if ((*it).Type == TimeLimit_Type && (*it).Value == TimeLimit_Value)
			{
				TimeLimit_out_Start = e.Timestamp;
				TimeRegionLimit_entered = true;
#if defined(DEBUG)
				cout << "ENTERING PRV REGION @ " << TimeLimit_out_Start << endl;
#endif
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
			currentRegion = new Region (e.Timestamp, TypeSeparator, ValueSeparator, 0);
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
						if (LookupCounter ((*it).Type, &index))
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
	if (TimeRegionLimit_entered && !TimeRegionLimit_exited)
	{
		for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
			if ((*it).Type == TimeLimit_Type && (*it).Value != TimeLimit_Value)
			{
				TimeLimit_out_End = e.Timestamp;
				TimeRegionLimit_exited = true;
#if defined(DEBUG)
				cout << "LEAVING PRV REGION @ " << TimeLimit_out_End << endl;
#endif
			}
	}
}

void ProcessFrameRegion::processEvent (struct singleevent_t &e)
{
	UNREFERENCED(e);
}

void ProcessFrameRegion::processCommunication (struct comm_t &c)
{
	UNREFERENCED(c);
}

void ProcessFrameRegion::setTimeRegionLimit (unsigned long long Type, unsigned long long Value)
{
	TimeLimit_Type = Type;
	TimeLimit_Value = Value;
	TimeRegionLimit_entered = TimeRegionLimit_exited = false;
}

} /* namespace libparaver */

using namespace::libparaver;
using namespace::std;

static vector<unsigned> convertCountersToID (vector<string> &lCounters, UIParaverTraceConfig *pcf)
{
	vector<unsigned> result;

	/* Look for every counter in the vector its code within the PCF file */
	for (unsigned i = 0; i < lCounters.size(); i++)
	{
		unsigned ctr = common::lookForCounter (lCounters[i], pcf);
		if (ctr == 0)
		{
			cerr << "FATAL ERROR! Cannot find counter " << lCounters[i] << " within the PCF file " << endl;
			exit (-1);
		}
		else
			result.push_back (ctr);
	}

	return result;
}

void SearchForRegionsWithinRegion (string tracename, unsigned task, unsigned thread,
	unsigned long long Type, unsigned long long TimeType, unsigned long long TimeValue,
	unsigned long long *out_Tstart, unsigned long long *out_Tend,
	vector<string> &vCounters, RegionInfo &regions,
	UIParaverTraceConfig *pcf)
{
	vector<unsigned> vIDCounters = convertCountersToID (vCounters, pcf);

	ProcessFrameRegion *p = new ProcessFrameRegion (tracename, true, task, thread, vIDCounters, Type);
	p->setTimeRegionLimit (TimeType, TimeValue);

	p->parseBody();

	*out_Tstart = p->TimeLimit_out_Start;
	*out_Tend = p->TimeLimit_out_End;
	regions.HWCnames = vCounters;
	regions.HWCcodes = vIDCounters;
	regions.foundRegions = p->foundRegions;
	for (list<Region*>::iterator i = regions.foundRegions.begin();
	  i != regions.foundRegions.end(); i++)
	{
		string tmp = pcf->getEventValue ((*i)->Type, (*i)->Value);
		if (tmp == "Not found" || tmp.length() == 0)
		{
			stringstream regionstream;
			regionstream << (*i)->Value;
			tmp = pcf->getEventType ((*i)->Type);
			if (tmp.length() > 0)
				tmp = tmp + "_" + regionstream.str();
			else
				tmp = string("Unknown_") + regionstream.str();
		}
		tmp = common::removeSpaces (tmp);
		(*i)->RegionName = tmp.substr (0, tmp.find_first_of (":[]{}() "));
		stringstream phasestr;
		phasestr << (*i)->Phase;
		(*i)->RegionName += "." + phasestr.str();
	}

#if defined(DEBUG)
	cout << "# Regions found = " << p->foundRegions.size() << " from " << p->TimeLimit_out_Start << " to " << p->TimeLimit_out_End << endl;
	for (list<Region *>::iterator i = regions.foundRegions.begin();
	  i != regions.foundRegions.end(); i++)
	{
		cout << "Found region " << (*i)->Type << ":" << (*i)->Value << " from " << (*i)->Tstart << " to " << (*i)->Tend << endl;
		cout << "COUNTERS:";
		vector<string>::iterator n = vCounters.begin();
		for (vector<unsigned long long>::iterator j = (*i)->HWCvalues.begin();
		  j != (*i)->HWCvalues.end(); j++, n++)
			cout << " "<< *n << "=>" << *j;
		cout << endl;
	}
#endif
}

