
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <string>

#include "ParaverTrace.h"
#include "ParaverTraceThread.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceApplication.h"
#include <UIParaverTraceConfig.h>

#include "FunctionBursts.h"

#include <list>
#include <map>

#define PAPI_INSTRUCTIONS 42000050
#define PAPI_MIN_VAL      42000000
#define PAPI_MAX_VAL      44000999

/* opcions */
#if 0
# define JUST_INSTRUCTIONS 
# define MONO_TASK_THREAD 
# define SELECTED_TASK    1 /* range 1..N */
# define SELECTED_THREAD  1 /* range 1..N */
# define DUMP_COUNTERS    1
#else
# define DUMP_COUNTERS    0
#endif

#include "process.H"

#include "globals.H"

void ReadCallerLinesIntoList (string file, string tracefile);

namespace domain {

list<unsigned long long> LlistaIDcallerlines;
list<unsigned long long> LlistaIDcaller;

bool check_counter_outlier = false;
bool check_time_outlier = false;
bool exclude_big_ipc = false;
bool exclude_1st = false;
float sigma_times = 2.0f;
list<unsigned long long> rutines_fold;
list<unsigned long long> rutines;

fstream outfile;

using namespace std;
using namespace domain;

double normalize_value (double value, double min, double max)
{
	return (value-min) / (max-min);
}

double denormalize_value (double normalized, double min, double max)
{
	return (normalized*(max-min))+min;
}


void Process::processMultiEvent (struct multievent_t &e)
{
	unsigned task = e.ObjectID.task - 1;
	unsigned thread = e.ObjectID.thread - 1;

	if (task >= ih.getNumTasks())
		return;

	TaskInformation *ti = ih.getTasksInformation();
	if (thread >= ti[task].getNumThreads())
		return;

	ThreadInformation *thi = ti[task].getThreadsInformation();

	for (vector<struct event_t>::iterator i = e.events.begin(); i != e.events.end(); i++)
	{
		if (thi->FirstIterationOcurred && ((*i).Type >= PAPI_MIN_VAL && (*i).Type <= PAPI_MAX_VAL))
		{
			if (thi->total_counters[thi->currentIteration-1].count ((*i).Type) == 0)
				thi->total_counters[thi->currentIteration-1][(*i).Type] = (*i).Value;
			else
				thi->total_counters[thi->currentIteration-1][(*i).Type] += (*i).Value;

			if (thi->FirstIterationOcurred && (*i).Type == PAPI_INSTRUCTIONS)
			{
				if (thi->lastTime != 0)
				{
					unsigned deltatime = e.Timestamp - thi->lastTime;
					thi->greatIPC[thi->currentIteration-1] =
					thi->greatIPC[thi->currentIteration-1] || ((1000*(*i).Value)/deltatime > 1800);
				}
				thi->lastTime = e.Timestamp;
			}
		}
		if (thi->FirstIterationOcurred && (*i).Type == phase_separator && thi->currentIteration > 0)
		{
			/* Add a phase if we found a phase separator */
			thi->numphases[thi->currentIteration-1]++;
			thi->phases[thi->currentIteration-1].push_back (e.Timestamp);
			thi->phases_from_iteration[thi->currentIteration-1].push_back (e.Timestamp-thi->lastIterationTime);
			thi->max_value_in_phase = MAX(thi->max_value_in_phase, (*i).Value);

			if (thi->currentIteration == chosen_iteration)
			{
				thi->max_numphase++;
				thi->phases_map.push_back ((*i).Value);
			}
		}
	}

	for (vector<struct event_t>::iterator i = e.events.begin(); i != e.events.end(); i++)
		if ((*i).Type == iteration_event)
		{
			if ((*i).Value == chosen_iteration)
				thi->timeChosenIteration = e.Timestamp;

			if (thi->currentIteration <= max_iteration && thi->currentIteration >= min_iteration && thi->currentIteration >= 1)
			{
				unsigned long long time = e.Timestamp - thi->lastIterationTime;
				thi->IterationTime[thi->currentIteration-1] = time;
			}

			if (thi->currentIteration > 0)
			{
				thi->phases[thi->currentIteration-1].push_back (e.Timestamp);
				thi->phases_from_iteration[thi->currentIteration-1].push_back (e.Timestamp-thi->lastIterationTime);
			}

			thi->FirstIterationOcurred = ((*i).Value >= 1);
			thi->lastIterationTime = e.Timestamp;
			thi->currentIteration = (*i).Value;
			thi->currentPhase = 0;

			if ((*i).Value != 0)
			{
				vector<unsigned long long> v1, v2;
				hwcmap c;

				v1.push_back (e.Timestamp);
				v2.push_back (0);
				thi->total_counters.push_back (c);
				thi->IterationTime.push_back (0);
				thi->greatIPC.push_back (false);
				thi->numphases.push_back (0);
				thi->phases.push_back (v1);
				thi->phases_from_iteration.push_back (v2);

#if 1
				/* Add starting phase */
				thi->numphases[thi->currentIteration-1]++;
				thi->phases[thi->currentIteration-1].push_back (e.Timestamp);
				thi->phases_from_iteration[thi->currentIteration-1].push_back (e.Timestamp-thi->lastIterationTime /* SHOULD BE 0! */);
				thi->max_value_in_phase = 0;
	
				if (thi->currentIteration == chosen_iteration)
				{
					thi->max_numphase++;
					thi->phases_map.push_back (0);
				}
#else
				thi->numphases[thi->currentIteration-1] = 0;
				thi->max_value_in_phase = 0;
#endif

			}
		}
}

} /* namespace domain */

using namespace domain;
using namespace std;

list<unsigned long long> ReadFile (char *file)
{
	list<unsigned long long> l;
  string line;
  ifstream f(file);

  if (f.is_open())
  {
    while (!f.eof())
    {
      getline (f, line);
			if (line.length() > 0)
				l.push_back (atoi(line.c_str()));
    }
    f.close();
  }
  else cout << "Unable to open file " << line; 

  return l;
}

int ProcessParameters (int argc, char *argv[])
{
	string lfunctions_file;

	if (argc < 2)
	{
		cerr << "Insufficient number of parameters" << endl;
		cerr << "Available options are: " << endl
		     << "-exclude-time-outlier/-include-time-outlier" << endl
		     << "-exclude-counter-outlier/-include-counter-outlier" << endl
		     << "-exclude-big-ipc" << endl
		     << "-exclude-1st" << endl
		     << "-num-out-samples K" << endl
		     << "-choose-iteration I" << endl
		     << "-list-functions FILE" << endl
		     << "-max-iteration K" << endl 
		     << "-min-iteration K" << endl 
		     << "-sigma F" << endl 
         << "-phase-separator P" << endl
         << "-dump-counters TASK(1..N) THREAD(1..N)" << endl;
		exit (-1);
	}

	for (int i = 1; i < argc-1; i++)
	{
		if (strcmp ("-exclude-time-outlier",  argv[i]) == 0)
		{
			check_time_outlier = true;
			continue;
		}
		else if (strcmp ("-include-time-outlier",  argv[i]) == 0)
		{
			check_time_outlier = false;
			continue;
		}
		else if (strcmp ("-exclude-counter-outlier",  argv[i]) == 0)
		{
			check_counter_outlier = true;
			continue;
		}
		else if (strcmp ("-include-counter-outlier",  argv[i]) == 0)
		{
			check_counter_outlier = false;
			continue;
		}
		else if (strcmp ("-exclude-big-ipc",  argv[i]) == 0)
		{
			exclude_big_ipc = true;
			continue;
		}
		else if (strcmp ("-exclude-1st",  argv[i]) == 0)
		{
			exclude_1st = true;
			continue;
		}
		else if (strcmp ("-choose-iteration", argv[i]) == 0)
		{
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid number of iteration" << endl;
			else
				chosen_iteration = atoi(argv[i]);
			continue;
		}
		else if (strcmp ("-iteration-event", argv[i]) == 0)
		{
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid number of event" << endl;
			else
				iteration_event = atoi(argv[i]);
			continue;
		}
		else if (strcmp ("-max-iteration", argv[i]) == 0)
		{
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid number of iterations" << endl;
			else
				max_iteration = atoi(argv[i]);
			continue;
		}
		else if (strcmp ("-min-iteration", argv[i]) == 0)
		{
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid number of iterations" << endl;
			else
				min_iteration = atoi(argv[i]);
			continue;
		}
		else if (strcmp ("-sigma", argv[i]) == 0)
		{
			i++;
			if (atof (argv[i]) == 0.0f)
				cerr << "Invalid sigma" << endl;
			else
				sigma_times = atof(argv[i]);
			continue;
		}
		else if (strcmp ("-phase-separator", argv[i]) == 0)
		{
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid separator" << endl;
			else
				phase_separator = atoi(argv[i]);
			continue;
		}
		else if (strcmp ("-list-functions", argv[i]) == 0)
		{
			i++;
			lfunctions_file = argv[i];
			/* rutines = ReadFile (argv[i]); */
		}
		else
			cerr << "unknown option " << argv[i] << endl;
	}

#if 0
	cout << "Parsing file " << argv[argc-1] << " from iteration " << min_iteration << " to " << max_iteration << endl;
	cout << "Iteration event = " << iteration_event <<  " Out iteration event = " << out_iteration_event << endl;
	cout << "Target iteration " << chosen_iteration << endl;
	cout << "Sigma = " << sigma_times << endl;
#endif

	if (lfunctions_file.length() > 0)
		::ReadCallerLinesIntoList (lfunctions_file, string(argv[argc-1]));

	return argc-1;
}


void ReadCallerLinesIntoList (string file, string tracefile)
{
	using namespace domain;

	string str;
	list<string> l_tmp;
	fstream file_op (file.c_str(), ios::in);

	if (!file_op.good())
	{
		cerr << "Error: Cannot open file " << file << endl;
		return;
	}

	while (file_op >> str)
	{
		/* Add element into list if it didn't exist */
		list<string>::iterator iter = find (l_tmp.begin(), l_tmp.end(), str);
		if (iter == l_tmp.end())
		{
			l_tmp.push_back (str);
			cout << "Adding function " << str << " in working set" << endl;
		}
	}
	file_op.close();

	string pcffile = tracefile.substr (0, tracefile.length()-3) + string ("pcf");
	domain::UIParaverTraceConfig *pcf = new domain::UIParaverTraceConfig (pcffile);
	vector<unsigned> v = pcf->getEventValuesFromEventTypeKey (30000000);
	unsigned i = 2;
	while (i != v.size())
	{
		string str = pcf->getEventValue(30000000, i);
		string func_name = str.substr (0, str.find(' '));
		if (find (l_tmp.begin(), l_tmp.end(), func_name) != l_tmp.end())
		{
			cout << "Adding identifier " << i << " for caller line " << pcf->getEventValue (30000000, i) << endl;
			LlistaIDcallerlines.push_back (i);
		}
		i++;
	}

	v = pcf->getEventValuesFromEventTypeKey (60000019);
	i = 2;
	while (i != v.size())
	{
		string str = pcf->getEventValue (60000019, i);
		if (find (l_tmp.begin(), l_tmp.end(), str) != l_tmp.end())
		{
			cout << "Adding identifier " << i << " for caller " << pcf->getEventValue (60000019, i) << endl;
			LlistaIDcaller.push_back (i);
		}
		i++;
	}

	delete pcf;
}


int main (int argc, char *argv[])
{
	string *tracefile;
	int res;

	res = ProcessParameters (argc, argv);

	tracefile = new string(argv[res]);

	Process *p = new Process (argv[res], true);
	vector<ParaverTraceApplication *> va = p->get_applications();
	if (va.size() != 1)
	{
		cerr << "ERROR Cannot parse traces with more than one application" << endl;
		return -1;
	}

	outfile.open (argv[res],ios::out|ios::app);

	p->listids = rutines;

	for (unsigned int i = 0; i < va.size(); i++)
	{
		vector<ParaverTraceTask *> vt = va[i]->get_tasks();
		p->ih.AllocateTasks (vt.size());
		TaskInformation *ti = p->ih.getTasksInformation();
		for (unsigned int j = 0; j < vt.size(); j++)
			ti[j].AllocateThreads(vt[j]->get_threads().size());
	}

	p->parseBody();

	if (exclude_big_ipc || check_time_outlier || check_counter_outlier)
	{
		for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
		{
			TaskInformation *ti = p->ih.getTasksInformation();
			for (unsigned j = 0; j < ti[i].getNumThreads(); j++) 
			{
				float sigma_counter, sigma_time;

				ThreadInformation *thi = ti[i].getThreadsInformation();
				unsigned long long tmp_counter = 0, tmp_time = 0;
				unsigned long long mitja_counter, mitja_time;
	
				for (unsigned int k = min_iteration-1; k < MIN(thi[j].total_counters.size(), max_iteration); k++)
				{
					tmp_counter += thi[j].total_counters[k][PAPI_INSTRUCTIONS];
					tmp_time    += thi[j].IterationTime[k];
				}
				mitja_counter = tmp_counter / (MIN(thi[j].total_counters.size(), max_iteration) - (min_iteration-1)); /* thi[j].total_counters.size(); */
				mitja_time    = tmp_time    / (MIN(thi[j].total_counters.size(), max_iteration) - (min_iteration-1)); /* thi[j].total_counters.size(); */
	
				tmp_counter = tmp_time = 0;
				for (unsigned int k = min_iteration-1; k < MIN(thi[j].total_counters.size(), max_iteration); k++)
				{
					tmp_counter += (thi[j].total_counters[k][PAPI_INSTRUCTIONS]-mitja_counter)*(thi[j].total_counters[k][PAPI_INSTRUCTIONS]-mitja_counter);
					tmp_time    += (thi[j].IterationTime[k]-mitja_time)*(thi[j].IterationTime[k]-mitja_time);
				}
				if (thi[j].total_counters.size()>1)
				{
					sigma_counter = sqrtf ((float) (tmp_counter)/(float) (thi[j].total_counters.size()-1));
					sigma_time    = sqrtf ((float) (tmp_time)   /(float) (thi[j].total_counters.size()-1));
				}
				else
					sigma_time = sigma_counter = 0.0f;
	
				for (unsigned int k = 0; k < min_iteration-1; k++)
					thi[j].excluded_iteration.push_back (true);

				for (unsigned int k = min_iteration-1; k < thi[j].total_counters.size(); k++)
				{
					bool excluded = (k==0)?exclude_1st:false;
					if (exclude_big_ipc)
						excluded |= thi[j].greatIPC[k];
					if (check_time_outlier)
						excluded |= fabs ((float)(thi[j].IterationTime[k])-(float)(mitja_time)) > (sigma_times*sigma_time);
					if (check_counter_outlier)
						excluded |= fabs ((float)(thi[j].total_counters[k][PAPI_INSTRUCTIONS])-(float)(mitja_counter)) > (sigma_times*sigma_counter);

					if (chosen_iteration-1 != k)
						thi[j].excluded_iteration.push_back (excluded);
					else
						thi[j].excluded_iteration.push_back (false);
				}
			}
		}
	}
	else
	{
		for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
		{
			TaskInformation *ti = p->ih.getTasksInformation();
			for (unsigned j = 0; j < ti[i].getNumThreads(); j++) 
			{
				ThreadInformation *thi = ti[i].getThreadsInformation();
				for (unsigned k = min_iteration-1; k < max_iteration; k++)
					thi[j].excluded_iteration.push_back (false);
			}
		}
	}

	TaskInformation *ti = p->ih.getTasksInformation();
	for (unsigned task = 0; task < p->ih.getNumTasks(); task++)
	{
		ThreadInformation *thi = ti[task].getThreadsInformation();
		for (unsigned thread = 0; thread < ti[task].getNumThreads(); thread++) 
		{
			unsigned long long refTimeIteration = thi[thread].IterationTime[chosen_iteration-1];

			for (unsigned i = 0; i < MIN(thi[thread].IterationTime.size(), max_iteration); i++)
			{
				if (!thi[thread].excluded_iteration[i])
					thi[thread].normalization_factor.push_back (((float)refTimeIteration/(float)thi[thread].IterationTime[i]));
				else
					thi[thread].normalization_factor.push_back (1.0f);
			}
		}
	}

#if 0
	/*** Check if all the iterations have the same number of MPI phases ***/
	for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
		for (unsigned j = 0; j < p->ih.getTasksInformation()[i].getNumThreads(); j++)
		{
			ThreadInformation *thi = p->ih.getTasksInformation()[i].getThreadsInformation();

			if (thi[j].numphases.size() > 0)
			{
				bool equal = true;
				for (unsigned k = 0; k < thi[j].numphases.size(); k++)
					equal = equal && thi[j].numphases[k] == thi[j].numphases[0];
			}
		}
#endif

	/*** Compute diff times for every MPI phases ***/
	for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
		for (unsigned j = 0; j < p->ih.getTasksInformation()[i].getNumThreads(); j++)
		{
			ThreadInformation *thi = p->ih.getTasksInformation()[i].getThreadsInformation();
			for (unsigned k = 0; k < thi[j].phases.size(); k++)
			{
				vector<unsigned long long> v;
				thi[j].phases_length.push_back(v);

				for (unsigned l = 1; l < (thi[j].phases)[k].size(); l++)
				{
					unsigned long long diff = ((thi[j].phases)[k])[l]-((thi[j].phases)[k])[l-1];
					((thi[j].phases_length)[k]).push_back (diff);
				}
			}
		}

	string traceprefix = tracefile->substr (0, tracefile->rfind ("."));
	p->DumpInformation (traceprefix);

	return 0;
}
