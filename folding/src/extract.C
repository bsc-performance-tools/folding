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

#define PAPI_MIN_COUNTER   42000000
//#define PAPI_MAX_COUNTER   42009998
#define PAPI_MAX_COUNTER   42999999

unsigned long long RegionSeparator = 1234567;
string RegionSeparatorName;
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
	vector<pair<event_t, unsigned long long> > vcallstack;

	unsigned long long CurrentIteration;
	unsigned long long CurrentPhase;
	unsigned long long CurrentRegion;
	unsigned long long StartRegion;

	unsigned long long LastCounterTime;

	void AllocateBufferCounters (int numCounters);
	ThreadInformation ();
};

ThreadInformation::ThreadInformation ()
{
	CurrentRegion = CurrentPhase = CurrentIteration = 0;
	LastCounterTime = 0;
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
	bool *CounterUsed;
	bool *HackCounter;
	unsigned long long LookupCounter (unsigned long long Counter, bool &found);
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

	pcf = new UIParaverTraceConfig;
	pcf->parse(pcffile);

	string s = pcf->getEventType (RegionSeparator);
	if (s.length() > 0)
		RegionSeparatorName = common::removeSpaces (s);
	else
		RegionSeparatorName = "Unknown";

	for (int i = PAPI_MIN_COUNTER; i <= PAPI_MAX_COUNTER; i++)
	{
		try
		{
			string s = pcf->getEventType (i);
			if (s.length() > 0)
				found_counters++;
		}
		catch (...)
		{ }
	}

	vector<unsigned> values = pcf->getEventValues(RegionSeparator);
	for (unsigned i = 0; i < values.size(); i++)
		TypeValuesLabels.push_back (pcf->getEventValue(RegionSeparator, values[i]));

	numCounterIDs = found_counters;
	CounterIDs = new unsigned long long[numCounterIDs];
	CounterIDNames = new string[numCounterIDs];
	HackCounter = new bool[numCounterIDs];
	CounterUsed = new bool[numCounterIDs];

	unsigned j = 0;
	for (unsigned i = PAPI_MIN_COUNTER; i <= PAPI_MAX_COUNTER; i++)
	{
		try
		{
			string s = pcf->getEventType (i);
			if (s.length() > 0)
			{
				CounterUsed[j] = false;
				CounterIDs[j] = i;
				CounterIDNames[j] = s.substr (s.find ('(')+1, s.find (')', s.find ('(')+1) - (s.find ('(') + 1));
				HackCounter[j] = (CounterIDNames[j] == "PM_CMPLU_STALL_FDIV" || CounterIDNames[j] == "PM_CMPLU_STALL_ERAT_MISS")?1:0;
				j++;
			}
		}
		catch (...)
		{ }
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
	
		vector<unsigned> v = pcf->getEventValues(30000000);
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

unsigned long long Process::LookupCounter (unsigned long long Counter, bool &found)
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

void Process::processMultiEvent (struct multievent_t &e)
{
	bool FoundSeparator = false;
	bool FoundPhaseSeparator = false;
	unsigned long long ValueSeparator = 0;
	int task = e.ObjectID.task - 1;
	int thread = e.ObjectID.thread - 1;

	//cout << "task = " << task << " IH.getNumTasks() = " << IH.getNumTasks() << endl;

	if (task >= IH.getNumTasks())
		return;

	TaskInformation *ti = IH.getTasksInformation();

	//cout << "thread = " << thread << " ti[task].getNumThreads() = " << ti[task].getNumThreads() << endl;

	if (thread >= ti[task].getNumThreads())
		return;

  ThreadInformation *thi = &((ti[task].getThreadsInformation())[thread]);

#if 0
	for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
		if ((*it).Type == 123456)
			thi->CurrentIteration = (*it).Value;
#endif

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
#if defined(DEBUG)
				cout << "Found counter @ " << e.Timestamp << endl;
#endif

				bool found;
				unsigned long long value;
				int index = LookupCounter ((*it).Type, found);

				if (found)
				{
					if (HackCounter[index])
					{
						value = (*it).Value;
						if (value > 0x80000000LL) /* If counter larger than 32 bits  (2^32) */
						{
							value = value & 0xffffffffLL;
							if ((value & 0x80000000LL) != 0)
								value = value ^ 0xffffffffLL;
							value = 0;
						}
					}
					else
						value = (*it).Value;

#if 0 /* HSG NEW */
					if (thi->CounterSamples[index].size() == 0 && thi->LastCounterTime > 0)
					{
//						cout << "current time = " << e.Timestamp << endl;
//						cout << "LastCounterTime = " << thi->LastCounterTime << endl;

						double rate = ((double) (value))/(e.Timestamp - thi->LastCounterTime);

//						cout << "calculated rate = " << rate << endl;

//						cout << "StartRegion = " << thi->StartRegion << endl;
//						cout << "delta(origin, current) = " << e.Timestamp - thi->StartRegion << endl;

						unsigned long long cextended = (e.Timestamp - thi->StartRegion) * rate;

//						cout << "cextended-pre = " << cextended << endl;

						thi->CounterSamples[index].push_back (cextended);
						CounterUsed[index] = true; /* do not mix, counteradded and counterused */
						CounterAdded = true;
					}
					else
					{
						thi->CounterSamples[index].push_back (value);
						CounterUsed[index] = true; /* do not mix, counteradded and counterused */
						CounterAdded = true;
					}
#endif
		
#if 1 /* HSG ORIGINAL */
#if defined(DEBUG)
					cout << "Adding value " << value << " for CounterIndex = " << index << endl;
#endif
					thi->CounterSamples[index].push_back (value);
					CounterUsed[index] = true; /* do not mix, counteradded and counterused */
					CounterAdded = true;

#endif
				} /* if found */
			}
		}

#if 0 /* HSG NEW */
		if (FoundSeparator && ValueSeparator == 0 && thi->CurrentRegion != 0 && !CounterAdded)
		{
			for (int i = PAPI_MIN_COUNTER; i <= PAPI_MAX_COUNTER; i++)
			{
				bool found;
				int cnt = LookupCounter (i, found);
				if (found)
					if (CounterUsed[cnt])
					{
//						cout << "last time = " << thi->TimeSamples.back() << endl;
//						cout << "last counter = " << thi->CounterSamples[cnt].back() << endl;
//						cout << "first time = " << thi->TimeSamples.front() << endl;

						unsigned long long totalcounter = 0;
						list<unsigned long long>::iterator it = thi->CounterSamples[cnt].begin();
						for (; it != thi->CounterSamples[cnt].end(); it++)
							totalcounter += *it;

						double rate = ((double) (totalcounter)) / (thi->TimeSamples.back()-thi->TimeSamples.front());

//						cout << "counter rate = " << rate << endl;

//						cout << "current time = " << e.Timestamp << endl;
//						cout << "delta(current,last) = " << e.Timestamp - thi->TimeSamples.back() << endl;

						unsigned long long cextended = (e.Timestamp - thi->TimeSamples.back())*rate;

//						cout << "cextended-post = " << cextended << endl;

						thi->CounterSamples[cnt].push_back (cextended);
					}
			}
			CounterAdded = true;
		}
#endif /* HSG NEW */

		if (CounterAdded)
		{
			thi->TimeSamples.push_back (e.Timestamp);
			thi->SkipSamples.push_back (skip);
		}

		bool found_min_level = false;
		unsigned MinCallerLevel = 0, MinCallerLevelValue = 0;

		for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
		{
			if ((*it).Type >= 30000000 && (*it).Type <= 30000199)
				thi->vcallstack.push_back (make_pair(*it,e.Timestamp));

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

#if defined(DEBUG)
		cout << "Final! thi->TimeSamples.size() " << thi->TimeSamples.size() << endl;
#endif

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
			string RegionNameValue = pcf->getEventValue (RegionSeparator, thi->CurrentRegion);

			/* Write the total time spent in this region */
			if (RegionNameValue.length() > 0 && RegionNameValue != "Not found")
			{
				thi->output << "T " << common::removeSpaces (RegionNameValue) << "." << thi->CurrentPhase << " " << TotalTime << endl;
			}
			else
			{
				thi->output << "T " << RegionSeparatorName << "_" << thi->CurrentRegion << "." << thi->CurrentPhase << " " << TotalTime << endl;
			}

			for (unsigned i = 0; i < thi->vcallstack.size(); i++)
			{
				double NTime = ::NormalizeValue (thi->vcallstack[i].second - thi->StartRegion, 0, TotalTime);
				thi->output << "C " << NTime << " "<< thi->vcallstack[i].first.Type << " " << thi->vcallstack[i].first.Value << endl;
			}

			/* Write total counters spent in this region */
			for (unsigned cnt = 0; cnt < numCounterIDs; cnt++)
			{
				if (!CounterUsed[cnt])
					continue;
				unsigned long long TotalCounter = 0;
				list<unsigned long long>::iterator Counter_iter = thi->CounterSamples[cnt].begin();
				for ( ; Counter_iter != thi->CounterSamples[cnt].end(); Counter_iter++)
					TotalCounter += (*Counter_iter);
				thi->output << "A " << CounterIDNames[cnt] << " " << TotalCounter << endl;
			}
		}

		for (unsigned cnt = 0; cnt < numCounterIDs; cnt++)
		{
			if (!CounterUsed[cnt])
				continue;

			unsigned long long TotalCounter = 0;
			list<unsigned long long>::iterator Counter_iter = thi->CounterSamples[cnt].begin();
			for ( ; Counter_iter != thi->CounterSamples[cnt].end(); Counter_iter++)
			{
				TotalCounter += (*Counter_iter);
			}

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
				//for (; Counter_iter_next != thi->CounterSamples[cnt].end();
				//	Counter_iter_next++, Times_iter++, Skip_iter++)
				for (; Counter_iter != thi->CounterSamples[cnt].end();
					Counter_iter++, Times_iter++, Skip_iter++)
				{
					if (!(*Skip_iter) && *Counter_iter > 0)
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
			else
			{
				/* Last pair is always 1 - 1, skip them! */
				//for (; Counter_iter_next != thi->CounterSamples[cnt].end();
				//	Counter_iter_next++, Counter_iter++, Times_iter++, Skip_iter++)
				for (; Counter_iter != thi->CounterSamples[cnt].end();
			 		Counter_iter++, Times_iter++, Skip_iter++)
				{
					if (!(*Skip_iter))
					{
						double NTime = ::NormalizeValue ((*Times_iter) - thi->StartRegion, 0, TotalTime);
						double NCounter = 0.0f;
	
#if defined(DEBUG)
						thi->output << "S " << CounterIDNames[cnt] << " / " <<  CounterIDs[cnt] << " " << NTime << " " << NCounter << " (TIME = " << (*Times_iter) << ", TOTALTIME= " << (*Times_iter) - thi->StartRegion<< "  )" << endl;
#else
						thi->output << "S " << CounterIDNames[cnt] << " " << NTime << " " << NCounter << " " << (*Times_iter) - thi->StartRegion << " " << 0.0f << " " << thi->CurrentIteration << endl;
#endif
					}

					/* Always! accumulate counters, even for skipped records! */
				}
				thi->CounterSamples[cnt].clear();
			}
			CounterUsed[cnt] = false; /* clean for next region */
		} /* Skip */

		thi->TimeSamples.clear();
		thi->SkipSamples.clear();

		/* Then dump caller line information */
#if 0
		for (list<LineCodeInformation*>::iterator it = thi->LineSamples.begin(); it != thi->LineSamples.end(); it++)
		{
			double NTime = ::NormalizeValue ((*it)->time - thi->StartRegion, 0, TotalTime);

#if defined(DEBUG)
			thi->output << "S LINE " << NTime << " " << (*it)->line << " at timestamp " << (*it)->time << " " << thi->CurrentIteration << endl;
			thi->output << "S LINEID " << NTime << " " << (*it)->lineid_value << " at timestamp " << (*it)->time << " " << thi->CurrentIteration << endl;
#else
			thi->output << "S LINE " << NTime << " " << (*it)->line << " " << (*it)->time << " 0 " << thi->CurrentIteration << endl;
			thi->output << "S LINEID " << NTime << " " << (*it)->lineid_value << " " << (*it)->time << " 0 " << thi->CurrentIteration << endl;
#endif
		}
#endif

		thi->LineSamples.clear();

	}

	/* If we found a phase separator, increase current phase */
	if (FoundPhaseSeparator)
	{
		thi->CurrentPhase++;
		thi->vcallstack.clear();
	}

	/* If we found a region separator, increase current region and reset the phase */
	if (FoundSeparator)
	{
		thi->CurrentRegion = ValueSeparator;
		thi->CurrentPhase = 0;
		thi->vcallstack.clear();
	}

	if ((FoundSeparator && ValueSeparator != 0) || FoundPhaseSeparator)
		thi->StartRegion = e.Timestamp;

	/* Annotate the time if we have found a counter in this event group */
	for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
		if ((*it).Type >= PAPI_MIN_COUNTER && (*it).Type <= PAPI_MAX_COUNTER)
		{
			thi->LastCounterTime = e.Timestamp;
			break;
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

/*
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
*/

	for (unsigned int i = 0; i < va.size(); i++)
	{
		vector<ParaverTraceTask *> vt = va[i]->get_tasks();
		p->IH.AllocateTasks (vt.size());
		TaskInformation *ti = p->IH.getTasksInformation();
		for (unsigned int j = 0; j < vt.size(); j++)
		{
			ti[j].AllocateThreads (vt[j]->get_threads()[0]->get_key());
			//ti[j].AllocateThreads (vt[j]->get_threads().size());
			for (unsigned int k = 0; k < ti[j].getNumThreads(); k++)
			{
				ThreadInformation *thi = &((ti[j].getThreadsInformation())[k]);
				thi->AllocateBufferCounters (p->numCounterIDs);

				stringstream tasknumber, threadnumber;
				tasknumber << j+1;
				threadnumber << k+1;
				string completefile = tracename.substr (0, tracename.length()-4) + ".extract." + tasknumber.str() + "." + threadnumber.str();
				thi->output.open (basename(completefile.c_str()));

#if 0
				/* Put definitions for the separator event */
				list<string>::iterator it = p->TypeValuesLabels.begin();
				for (; it != p->TypeValuesLabels.end(); it++)
					thi->output << "D " << common::removeSpaces (*it) << endl;
#endif
			}
		}
  }

	p->parseBody();

	for (unsigned int i = 0; i < va.size(); i++)
	{
		vector<ParaverTraceTask *> vt = va[i]->get_tasks();
		TaskInformation *ti = p->IH.getTasksInformation();
		for (unsigned int j = 0; j < vt.size(); j++)
			for (unsigned int k = 0; k < ti[j].getNumThreads(); k++)
			{
				ThreadInformation *thi = &((ti[j].getThreadsInformation())[k]);
				thi->output.close();
			}
	}

	string ControlFile = tracename.substr (0, tracename.length()-4) + ".control";
	ofstream cfile (basename(ControlFile.c_str()));
	cfile << tracename << endl;
	cfile << RegionSeparator << endl;
	cfile << RegionSeparatorName << endl;
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

