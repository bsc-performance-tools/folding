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

#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

namespace libparaver {

class ThreadInformation
{
	public:
	struct multievent_t LastEvents;
	bool hasLastEvents;

	ThreadInformation ();
};

ThreadInformation::ThreadInformation()
{
	LastEvents.Timestamp = 0;
	hasLastEvents = false;
}

class TaskInformation
{
	private:
	int numThreads;
	ThreadInformation *ThreadsInfo;

	public:
	int getNumThreads (void) const
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
	int getNumTasks (void) const
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

	public:
	Process (string prvFile, bool multievents, string tracename);
	~Process ();

	void processState (struct state_t &s);
	void processMultiEvent (struct multievent_t &e);
	void processEvent (struct singleevent_t &e);
	void processCommunication (struct comm_t &c);
	void processCommunicator (string &c);
	void processComment (string &c);
	void processLastEvents (void);
	void processLastEvents2 (void);
	InformationHolder ih;
};

Process::Process (string prvFile, bool multievents, string tracename) : ParaverTrace (prvFile, multievents)
{
	traceout.open (tracename.c_str());
	if (!traceout.is_open())
	{
		cout << "Unable to create " << tracename << endl;
		exit (-1);
	}
}

Process::~Process ()
{
	traceout.close ();
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
	int task = e.ObjectID.task - 1;
	int thread = e.ObjectID.thread - 1;

	if (task >= ih.getNumTasks())
		return;

	TaskInformation *ti = ih.getTasksInformation();
	if (thread >= ti[task].getNumThreads())
		return;

	ThreadInformation *thi = &((ti[task].getThreadsInformation())[thread]);

	if (thi->LastEvents.Timestamp < e.Timestamp)
	{
		/* If the timestamp is in the future, dump older accumulated events */
		if (thi->LastEvents.events.size() > 0)
		{
			traceout << "2:" 
				<< e.ObjectID.cpu << ":" << e.ObjectID.ptask << ":" << e.ObjectID.task << ":" << e.ObjectID.thread << ":"
				<< thi->LastEvents.Timestamp;

				/* Add events to a multimap that they are not replicated inside the tracefile (applied to clusters?) */
			multimap<unsigned, unsigned long long> mm_filter;
			for (const auto &e : thi->LastEvents.events)
			{
				bool found = false;
				multimap<unsigned, unsigned long long>::iterator it;
				pair<multimap<unsigned, unsigned long long>::iterator,multimap<unsigned, unsigned long long>::iterator> ret = mm_filter.equal_range (e.Type);
				for (it = ret.first; it != ret.second; ++it)
					found = found || (*it).second == e.Value;

				if (!found)
					mm_filter.insert (make_pair (e.Type, e.Value));
			}
			for (const auto &e : mm_filter)
				traceout << ":" << e.first << ":" << e.second;
			traceout << endl;
		}

		thi->LastEvents = e;
		thi->hasLastEvents = true;
	}
	else if (thi->LastEvents.Timestamp == e.Timestamp)
	{
		/* If are the same timestamp, join in a single multievent */
		for (const auto & event : e.events)
			thi->LastEvents.events.push_back (event);
	}
	else if (thi->LastEvents.Timestamp > e.Timestamp)
	{
		/* This should not happen! */
		cout << "Badly ordered trace. Task " << e.ObjectID.task << " thread "
			<< e.ObjectID.thread << " at times " << e.Timestamp << " and " 
			<< thi->LastEvents.Timestamp << endl;
		exit (-1);
	}
}

void Process::processLastEvents (void)
{
	TaskInformation *ti = ih.getTasksInformation();
	for (int task = 0; task < ih.getNumTasks(); task++)
		for (int thread = 0; thread < ti[task].getNumThreads(); thread++)
		{
			ThreadInformation *thi = &((ti[task].getThreadsInformation())[thread]);
			if (thi->hasLastEvents && thi->LastEvents.events.size() > 0)
			{
				traceout << "2:" 
					<< thi->LastEvents.ObjectID.cpu << ":"
					<< thi->LastEvents.ObjectID.ptask << ":"
					<< thi->LastEvents.ObjectID.task << ":"
					<< thi->LastEvents.ObjectID.thread << ":"
					<< thi->LastEvents.Timestamp;

				/* Add events to a multimap that they are not replicated inside the tracefile (applied to clusters?) */
				multimap<unsigned, unsigned long long> mm_filter;
				for (const auto &e : thi->LastEvents.events)
				{
					bool found = false;
					multimap<unsigned, unsigned long long>::iterator it;
					pair<multimap<unsigned, unsigned long long>::iterator,multimap<unsigned, unsigned long long>::iterator> ret = mm_filter.equal_range (e.Type);
					for (it = ret.first; it != ret.second; ++it)
						found = found || (*it).second == e.Value;

					if (!found)
						mm_filter.insert (make_pair (e.Type, e.Value));
				}
				for (const auto &e : mm_filter)
					traceout << ":" << e.first << ":" << e.second;
				traceout << endl;
			}
		}
}

void Process::processEvent (struct singleevent_t &)
{
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

} /* namespace libparaver */

using namespace::libparaver;
using namespace::std;

int main (int argc, char *argv[])
{
	cout << "Folding (fuse) based on branch " FOLDING_SVN_BRANCH " revision " << FOLDING_SVN_REVISION << endl;

	string tracename;

	if (argc != 2)
	{
		cerr << "You must provide a tracefile!" << endl;
		return -1;
	}

	tracename = string(argv[1]);
	if (!common::existsFile(tracename))
	{
		cerr << "The tracefile " << tracename << " does not exist!" << endl;
		return -2;
	}

	string bfileprefix = common::basename (tracename.substr (0, tracename.rfind(".prv")));

	Process *p = new Process (argv[1], true, (bfileprefix + string(".fused.prv")).c_str());

	vector<ParaverTraceApplication *> va = p->get_applications();
	if (va.size() != 1)
	{
		cerr << "ERROR Cannot parse traces with more than one application" << endl;
		return -1;
	}

	for (unsigned int i = 0; i < va.size(); i++)
	{
		vector<ParaverTraceTask *> vt = va[i]->get_tasks();
		p->ih.AllocateTasks (vt.size());
		TaskInformation *ti = p->ih.getTasksInformation();
		for (unsigned int j = 0; j < vt.size(); j++)
			ti[j].AllocateThreads (vt[j]->get_threads().size());
	}

	p->parseBody();

	p->processLastEvents();

	/* Copy .pcf and .row files */
	ifstream ifs_pcf ((tracename.substr (0, tracename.rfind(".prv"))+string(".pcf")).c_str());
	if (ifs_pcf.is_open())
	{
		ofstream ofs_pcf ( (bfileprefix + string (".fused.pcf")).c_str() );
		ofs_pcf << ifs_pcf.rdbuf();
		ifs_pcf.close();
		ofs_pcf.close();
	}

	ifstream ifs_row ((tracename.substr (0, tracename.rfind(".prv"))+string(".row")).c_str());
	if (ifs_row.is_open())
	{
		ofstream ofs_row ( (bfileprefix + string (".fused.row")).c_str() );
		ofs_row << ifs_row.rdbuf();
		ifs_row.close();
		ofs_row.close();
	}

	return 0;
}

