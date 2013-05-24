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

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <string.h>
#include <list>

#include "region-analyzer-first-occurrency.H"

#define PAPI_MIN_COUNTER   42000000
#define PAPI_MAX_COUNTER   42999999

namespace libparaver {

class ProcessFirstOccurrence : public ParaverTrace
{
	private:
	Region *currentRegion;
	unsigned *CounterIDs;

	unsigned long long Separator;
	unsigned task;
	unsigned thread;
	unsigned numCounterIDs;
	unsigned CurrentPhase;
	vector<unsigned long long> phasetypes;
	
	bool LookupCounter (unsigned Counter, unsigned *index);

	public:
	list<Region*> foundRegions;

	ProcessFirstOccurrence (string prvFile, bool multievents, int task, int thread, vector<unsigned> &CounterList, unsigned long long Separator, vector<unsigned long long> &phasetypes);

	void processState (struct state_t &s);
	void processMultiEvent (struct multievent_t &e);
	void processEvent (struct singleevent_t &e);
	void processCommunication (struct comm_t &c);
	void processCommunicator (string &c);
	void processComment (string &c);
};

ProcessFirstOccurrence::ProcessFirstOccurrence (string prvFile, bool multievents, int task, int thread, vector<unsigned> &CounterList, unsigned long long Separator, vector<unsigned long long> & phasetypes) : ParaverTrace (prvFile, multievents)
{
	numCounterIDs = CounterList.size();
	CounterIDs = new unsigned [numCounterIDs];

	for (unsigned i = 0; i < CounterList.size(); i++)
		CounterIDs[i] = CounterList[i];

	currentRegion = NULL;
	this->Separator = Separator;
	this->task = task;
	this->thread = thread;
	CurrentPhase = 0;
	this->phasetypes = phasetypes;
}

void ProcessFirstOccurrence::processComment (string &c)
{
	UNREFERENCED(c);
}

void ProcessFirstOccurrence::processCommunicator (string &c)
{
	UNREFERENCED(c);
}

void ProcessFirstOccurrence::processState (struct state_t &s)
{
	UNREFERENCED(s);
}

bool ProcessFirstOccurrence::LookupCounter (unsigned Counter, unsigned *index)
{
	for (unsigned i = 0; i < numCounterIDs; i++)
		if (CounterIDs[i] == Counter)
		{
			*index = i;
			return true;
		}

	return false;
}

void ProcessFirstOccurrence::processMultiEvent (struct multievent_t &e)
{
	bool FoundSeparator = false;
	bool FoundPhaseSeparator = false;
	unsigned long long ValueSeparator = 0;

	if (e.ObjectID.task != task || e.ObjectID.thread != thread)
		return;

	for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
	{
		if ((*it).Type == Separator)
		{
			FoundSeparator = true;
			ValueSeparator = (*it).Value;
		}
		FoundPhaseSeparator = find (phasetypes.begin(), phasetypes.end(), (*it).Type) != phasetypes.end() || FoundPhaseSeparator;
	}

	if (FoundSeparator && ValueSeparator != 0)
	{
		/* Start a region */
		CurrentPhase = 0;
		currentRegion = new Region (e.Timestamp, Separator, ValueSeparator, CurrentPhase);
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

	if ((FoundPhaseSeparator || (FoundSeparator && ValueSeparator == 0)) && currentRegion != NULL)
	{
		/* Complete a region and add it only if it's the first region with this
		   pair type/value */
		currentRegion->Tend = e.Timestamp;

		list<Region *>::iterator i = foundRegions.begin();
		bool found = false;
		for (; i != foundRegions.end() && !found; i++)
			found = ((*i)->Value == currentRegion->Value && (*i)->Phase == currentRegion->Phase);

		if (!found)
			foundRegions.push_back (currentRegion);

		if (FoundSeparator && ValueSeparator == 0)
		{
			currentRegion = NULL;
		}
		else if (FoundPhaseSeparator)
		{
			/* Start a region */
			CurrentPhase++;

			currentRegion = new Region (e.Timestamp, currentRegion->Type, currentRegion->Value, CurrentPhase);
			for (unsigned i = 0; i < numCounterIDs; i++)
				currentRegion->HWCvalues.push_back (0);
		}
	}
}

void ProcessFirstOccurrence::processEvent (struct singleevent_t &e)
{
	UNREFERENCED(e);
}

void ProcessFirstOccurrence::processCommunication (struct comm_t &c)
{
	UNREFERENCED(c);
}

} /* namespace libparaver */


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

void SearchForRegionsFirstOccurrence (string tracename, unsigned task, unsigned thread,
	unsigned long long Type, vector<string> &vCounters, RegionInfo &regions,
	UIParaverTraceConfig *pcf, vector<unsigned long long> &phasetypes)
{
	vector<unsigned> vIDCounters = convertCountersToID (vCounters, pcf);

	ProcessFirstOccurrence *p = new ProcessFirstOccurrence (tracename, true, task, thread, vIDCounters, Type, phasetypes);

	p->parseBody();

	regions.HWCnames = vCounters;
	regions.HWCcodes = vIDCounters;
	regions.foundRegions = p->foundRegions;
	for (list<Region*>::iterator i = regions.foundRegions.begin();
	  i != regions.foundRegions.end(); i++)
	{
		try
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
			//(*i)->RegionName = tmp.substr (0, tmp.find_first_of (":[]{}() "));
			(*i)->RegionName = tmp;
			stringstream phasestr;
			phasestr << (*i)->Phase;
			(*i)->RegionName += "." + phasestr.str();
		}
		catch (...)
		{
		}
	}

#if defined(DEBUG)
	cout << "# Regions found = " << p->foundRegions.size() << endl;
	for (list<Region *>::iterator i = regions.foundRegions.begin();
	  i != regions.foundRegions.end(); i++)
	{
		cout << "Found region " << (*i)->Type << ":" << (*i)->Value << " from " << (*i)->Tstart << " to " << (*i)->Tend << endl;
		cout << "COUNTERS:";
		vector<string>::iterator n = vCounters.begin();
		for (vector<unsigned long long>::iterator j = (*i)->HWCvalues.begin(); j != (*i)->HWCvalues.end(); j++, n++)
			cout << " "<< *n << "=>" << *j;
		cout << endl;
	}
#endif
}

