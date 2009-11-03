
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <string>

#include "kriger_wrapper.h"

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

#define USE_MIN_TIME_ITERATION
// #define USE_MAX_TIME_ITERATION


#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

void ReadCallerLinesIntoList (string file, string tracefile);

namespace domain {

list<unsigned long long> LlistaIDcallerlines;
list<unsigned long long> LlistaIDcaller;

unsigned int chosen_iteration = 1;

unsigned int iteration_event = 123456;
unsigned int out_iteration_event = 1234567;

bool dump_splitted_counters = false;

bool dump_counters_gnuplot = false;
unsigned dump_counters_gnuplot_thread, dump_counters_gnuplot_task;

int num_out_samples = 1000;
unsigned numpartitions = 100;
unsigned max_iteration = 1000, min_iteration = 1;
bool check_counter_outlier = false;
bool check_time_outlier = false;
bool exclude_big_ipc = false;
bool exclude_1st = false;
float sigma_times = 2.0f;
list<unsigned long long> rutines_fold;
list<unsigned long long> rutines;

fstream outfile;

using namespace std;

class TimeValue
{
	public:
	unsigned long long realvalue;
	unsigned long long value;
	unsigned long long time;
	unsigned long long phase_time;
	unsigned long long iteration;
	unsigned long long absolute;
	unsigned long long phase;
	unsigned sample_address;
	unsigned sample_call;
	float normalized_time;
	float normal;
};

class Timelisted_HWcounter
{
	public:
	list<TimeValue> timevalue;
	unsigned long long lastreference;
	
	Timelisted_HWcounter ()
	{
		lastreference = 0;
	};
};

typedef map<unsigned long long, Timelisted_HWcounter, less<unsigned long long> > timelisted_hwcmap;
typedef map<unsigned long long, unsigned long long, less<unsigned long long> > hwcmap;

class ThreadInformation
{
	public:
 	timelisted_hwcmap timelisted_hwcounters;
	vector<float> normalization_factor;
	vector<unsigned long long> maxTimes;
	vector<unsigned long long> minTimes;

	vector<unsigned long long> numMPIs;
	vector<vector<unsigned long long> > MPIphases;
	vector<vector<unsigned long long> > MPIphases_from_iteration;
	vector<vector<unsigned long long> > MPIphases_length;
	vector<vector<unsigned long long> > MPIphases_outsamples;
	vector<vector<hwcmap> > MPIphases_total_counters;
	vector<vector<float> > MPIphases_factor;
	unsigned current_MPIphase;
	unsigned long long current_MPIphase_time;

	vector<hwcmap> total_counters; /* map of counters at the end of each iteration */
#if 0
	vector<hwcmap> phase_counters; /* map of counters at the end of each phase -- on target iteration */
#endif
	vector<bool> excluded_iteration;
	vector<bool> greatIPC;

	FunctionBursts fb;

	unsigned long long maxTimeIteration;
	unsigned long long minTimeIteration;
	unsigned long long currentIteration;
	unsigned long long currentPhase;
	unsigned long long lastTime;
	unsigned long long lastIterationTime;
	unsigned long long timeChosenIteration;
	bool FirstIterationOcurred;
	bool inMPI;
	float sigma_time;
	float sigma_counter;

	ThreadInformation ()
	{ 
		FirstIterationOcurred = inMPI = false;
		lastTime = lastIterationTime = currentPhase = currentIteration = maxTimeIteration = timeChosenIteration = 0;
		minTimeIteration = 0xffffffffffffffffULL;
		current_MPIphase = current_MPIphase_time = 0;
	};
};

class TaskInformation
{
	private:
	unsigned numThreads;
	ThreadInformation *ThreadsInfo;

	public:
	unsigned getNumThreads (void)
	{ return numThreads; };

	ThreadInformation* getThreadsInformation (void)
	{ return ThreadsInfo; };

	void AllocateThreads (unsigned numThreads);
	~TaskInformation();
};

TaskInformation::~TaskInformation()
{
	for (unsigned i = 0; i < numThreads; i++)
		delete [] ThreadsInfo;
}

void TaskInformation::AllocateThreads (unsigned numThreads)
{
	this->numThreads = numThreads;
	ThreadsInfo = new ThreadInformation[this->numThreads];
}

class InformationHolder
{
	private:
	unsigned numTasks;
	TaskInformation *TasksInfo;
	
	public:
	unsigned getNumTasks (void)
	{ return numTasks; };

	TaskInformation* getTasksInformation (void)
	{ return TasksInfo; };

	void AllocateTasks (unsigned numTasks);
	~InformationHolder();
};

InformationHolder::~InformationHolder()
{
	for (unsigned i = 0; i < numTasks; i++)
		delete [] TasksInfo;
}

void InformationHolder::AllocateTasks (unsigned numTasks)
{
	this->numTasks = numTasks;
	TasksInfo = new TaskInformation[this->numTasks];
}

class Process : public ParaverTrace
{
	public:
	Process (string prvFile, bool multievents);

	list<unsigned long long> listids;

	void processMultiEvent (struct multievent_t &e);

	void DumpParaverLine (fstream &f, unsigned long long type, unsigned long long value, unsigned long long time, unsigned long long task, unsigned long long thread);

	void dumpCounters_entireIteration (unsigned task, unsigned thread);
	void dumpCounters_splitIteration (unsigned task, unsigned thread);

	unsigned PROCESS;

	InformationHolder ih;
	~Process()
	{ };
};

Process::Process (string prvFile, bool multievents) : ParaverTrace (prvFile, multievents)
{
}

double normalize_value (double value, double min, double max)
{
	return (value-min) / (max-min);
}

double denormalize_value (double normalized, double min, double max)
{
	return (normalized*(max-min))+min;
}


void Process::DumpParaverLine (fstream &f, unsigned long long type, unsigned long long value, unsigned long long time, unsigned long long task, unsigned long long thread)
{
	/* 2:14:1:14:1:69916704358:40000003:0 */
	f << "2:" << task << ":1:" << task << ":" << thread << ":" << time << ":" << type << ":" << value << endl;
}

void Process::processMultiEvent (struct multievent_t &e)
{
	bool in_mpi = false, in_uf = false;
	unsigned task = e.ObjectID.task - 1;
	unsigned thread = e.ObjectID.thread - 1;

	if (task >= ih.getNumTasks())
		return;

	TaskInformation *ti = ih.getTasksInformation();
	if (thread >= ti[task].getNumThreads())
		return;

	ThreadInformation *thi = &((ti[task].getThreadsInformation())[thread]);

	if (PROCESS == 1)
	{
	/* Check consistency */
	if (task < ih.getNumTasks())
	{
		if (thread < ti[task].getNumThreads())
		{
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
				if (thi->FirstIterationOcurred && (*i).Type == 60000019 && thi->currentIteration > 0)
				{
					thi->numMPIs[thi->currentIteration-1]++;
					thi->MPIphases[thi->currentIteration-1].push_back (e.Timestamp);
					thi->MPIphases_from_iteration[thi->currentIteration-1].push_back (e.Timestamp-thi->lastIterationTime);
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
						thi->maxTimes[thi->currentIteration-1] = time;
						thi->minTimes[thi->currentIteration-1] = time;
						thi->maxTimeIteration = MAX(time, thi->maxTimeIteration);
					}

					if (thi->currentIteration > 0)
					{
						thi->MPIphases[thi->currentIteration-1].push_back (e.Timestamp);
						thi->MPIphases_from_iteration[thi->currentIteration-1].push_back (e.Timestamp-thi->lastIterationTime);
					}

					thi->FirstIterationOcurred = ((*i).Value >= 1);
					thi->lastIterationTime = e.Timestamp;
					thi->currentIteration = (*i).Value;
					thi->currentPhase = 0;

#if 0
					if (thi->currentIteration == chosen_iteration)
					{
						hwcmap mapofcounters;
						thi->phase_counters.push_back (mapofcounters);
					}
#endif

					if ((*i).Value != 0)
					{
						vector<unsigned long long> v1, v2;
						hwcmap c;

						v1.push_back (e.Timestamp);
						v2.push_back (0);
						thi->total_counters.push_back (c);
						thi->maxTimes.push_back (0);
						thi->minTimes.push_back (0);
						thi->greatIPC.push_back (false);
						thi->numMPIs.push_back (0);
						thi->MPIphases.push_back (v1);
						thi->MPIphases_from_iteration.push_back (v2);
					}
				}

#if 0
				else if ((*i).Type >= 30000000 && (*i).Type <= 30000000+100 && thi->currentIteration > 0)
				{
					if (thi->currentIteration <= maxiterations)
					{
					unsigned long long time = e.Timestamp - thi->lastIterationTime + thi->timeFirstIteration;
					thi->maxTimes[thi->currentIteration-1] = MAX(time, thi->maxTimes[thi->currentIteration-1]);
					thi->maxTimeIteration = MAX(time, thi->maxTimeIteration);
					thi->minTimeIteration = MIN(time, thi->minTimeIteration);
					}
				}
#endif
		}
	}
	} /* PROCESS == 1 */
	else if (PROCESS == 2)
	{
	bool has_reset = false;

	/* Check consistency */
	if (task < ih.getNumTasks())
	{
		unsigned long long type;
		unsigned long long value;

		if (thread < ti[task].getNumThreads())
		{

#if 0
			/* Calculem quines rutines de les triades son al callstack (seleccionem
			   la que esta mes aprop del top de la pila) */

			bool found = false;
			unsigned long long mintype;
			unsigned long long minvalue_for_type;

			for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
			{
				type = (*it).Type;
				value = (*it).Value;


				if (thi->FirstIterationOcurred && (type >= 30000000 && type <= 30000100) && !thi->excluded_iteration[thi->currentIteration-1] && thi->currentIteration <= max_iteration && thi->currentIteration >= min_iteration)
				{
					unsigned long long t = e.Timestamp - thi->lastIterationTime + thi->timeChosenIteration; /* HSG TO CHECK */
					DumpParaverLine (outfile, 37000000 + (type-30000000), value, t, task+1, thread+1);

					list<unsigned long long>::iterator it = find (rutines_fold.begin(), rutines_fold.end(), value);
					if (it != rutines_fold.end())
					{
						if (found)
						{
							if (mintype > type)
							{
								minvalue_for_type = value;
								mintype = type;
							}
						}
						else
						{
							minvalue_for_type = value;
							mintype = type;
							found = true;
						}
					}
				}
			}

				if (found && thi->FirstIterationOcurred && !thi->excluded_iteration[thi->currentIteration-1] && thi->currentIteration <= max_iteration && thi->currentIteration >= min_iteration)
				{
					unsigned long long t = e.Timestamp - thi->lastIterationTime + thi->timeChosenIteration; /* HSG TO CHECK */
					DumpParaverLine (outfile, 36000000, minvalue_for_type, t, task+1, thread+1);
				}
#endif


			/* Are we in a user function event or in a MPI event? */
			in_mpi = false;
			in_uf = false;
			for (vector<struct event_t>::iterator i = e.events.begin(); i != e.events.end(); i++)
			{
				in_mpi = in_mpi || ((*i).Type == 50000001 || (*i).Type == 50000002 || (*i).Type == 50000003);
				in_uf = in_uf || (*i).Type == 60000019;
			}

			bool has_sample = false;
			unsigned top_callstack_sample;
			unsigned top_callstack_sample_level;
			unsigned top_callstack_call;

			/* Get the top of the known stack -- known as user provide */
			for (vector<struct event_t>::iterator i = e.events.begin(); i != e.events.end(); i++)
				if ((*i).Type >= 30000000 && (*i).Type <= 30000100)
				{
					if (find (LlistaIDcallerlines.begin(), LlistaIDcallerlines.end(), (*i).Value) != LlistaIDcallerlines.end())
					{
						if (has_sample)
						{
							if (top_callstack_sample_level > (*i).Type)
							{
								top_callstack_sample_level = (*i).Type;
								top_callstack_sample = (*i).Value;
							}
						}
						else
						{
							has_sample = true;
							top_callstack_sample = (*i).Value;
							top_callstack_sample_level = (*i).Type;
						}
					}
				}

			top_callstack_call = 0;
			for (vector<struct event_t>::iterator i = e.events.begin(); i != e.events.end(); i++)
				if ((*i).Type == top_callstack_sample_level + 100000)
					if (find (LlistaIDcaller.begin(), LlistaIDcaller.end(), (*i).Value) != LlistaIDcaller.end())
						top_callstack_call = (*i).Value;

#if 0
			if (!in_uf && !in_mpi)
			{
				cout << "EVENTS";
				for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
					cout << "[" << (*it).Type << "," << (*it).Value << "]";
				cout << endl;
			}
#endif

			for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
			{
				type = (*it).Type;
				value = (*it).Value;

				if (thi->FirstIterationOcurred && (type >= PAPI_MIN_VAL && type <= PAPI_MAX_VAL) && thi->currentIteration <= max_iteration && thi->currentIteration >= min_iteration && !in_mpi && !in_uf )
				{
					if (!thi->excluded_iteration[thi->currentIteration-1])
					{
						TimeValue tv;
						bool inserted = false;

						tv.sample_address =  (has_sample)?top_callstack_sample:0;
						tv.sample_call = (has_sample)?top_callstack_call:0;

#define MPI_PHASES_TIME

#if !defined(MPI_PHASES_TIME)

						tv.time = e.Timestamp - thi->lastIterationTime;

# if defined(USE_MAX_TIME_ITERATION)
						tv.normalized_time = (thi->normalization_factor[thi->currentIteration-1]*(e.Timestamp - thi->lastIterationTime))/((float)thi->maxTimeIteration); 
# elif defined(USE_MIN_TIME_ITERATION)
						tv.normalized_time = (thi->normalization_factor[thi->currentIteration-1]*(e.Timestamp - thi->lastIterationTime))/((float)thi->minTimeIteration); 
# else
#  error "Define USE_MAX_TIME_ITERATION or USE_MIN_TIME_ITERATION"
# endif

#elif defined(MPI_PHASES_TIME)

						tv.time = e.Timestamp - thi->lastIterationTime;
						tv.normalized_time = (thi->normalization_factor[thi->currentIteration-1]*(e.Timestamp - thi->lastIterationTime))/((float)thi->maxTimeIteration);
						tv.normal = (float)(e.Timestamp - thi->current_MPIphase_time)/(float)(thi->MPIphases_length[thi->currentIteration-1][thi->current_MPIphase]);
						tv.absolute = e.Timestamp;
#endif

						tv.phase = thi->current_MPIphase;

						if (!has_reset)
						{
							tv.value = thi->timelisted_hwcounters[type].lastreference + value;
							tv.realvalue = value;
						}
						else
							tv.value = tv.realvalue = 0;
						tv.iteration = thi->currentIteration;
#if 0	
						if (task == 0 && thread == 0 && type == PAPI_INSTRUCTIONS)
						fprintf (stdout, "it %d ph %d timestamp = %llu mpi_phase_time = %llu  (diff %llu) length = %llu normal = %f countervalue=%llu counterrealvalue=%llu\n", thi->currentIteration, thi->current_MPIphase, e.Timestamp, thi->current_MPIphase_time, e.Timestamp - thi->current_MPIphase_time, thi->MPIphases_length[thi->currentIteration-1][thi->current_MPIphase], tv.normal, tv.value, tv.realvalue);
#endif

						for (list<TimeValue>::iterator i = thi->timelisted_hwcounters[type].timevalue.begin();
							i != thi->timelisted_hwcounters[type].timevalue.end() && !inserted;
							i++)
							if ((*i).normalized_time > tv.normalized_time)
							{
								thi->timelisted_hwcounters[type].timevalue.insert (i, tv);
								inserted = true;
							}
						if (!inserted)
							thi->timelisted_hwcounters[type].timevalue.push_back (tv);

						thi->timelisted_hwcounters[type].lastreference = tv.value;

#if DUMP_COUNTERS
#if defined(MONO_TASK_THREAD )
						if (thread == SELECTED_THREAD-1 && task == SELECTED_TASK-1 && type == PAPI_INSTRUCTIONS)
#endif
							cout << task << "," << thread << " TYPE " << type << " VALUE " << value << " ACCUMULATED " << thi->timelisted_hwcounters[type].lastreference << " TIME " << tv.normalized_time << " ITERATION " << thi->currentIteration << endl;
#endif
					}
				}
				else if (thi->FirstIterationOcurred && (type >= PAPI_MIN_VAL && type <= PAPI_MAX_VAL) && thi->currentIteration <= max_iteration && thi->currentIteration >= min_iteration /* && in_mpi */ )
				{
					if (!thi->excluded_iteration[thi->currentIteration-1])
						thi->timelisted_hwcounters[type].lastreference += value;
				}
			}


			if (thi->FirstIterationOcurred && thi->currentIteration > 0 && in_uf)
			{
				for (timelisted_hwcmap::iterator j = thi->timelisted_hwcounters.begin(); j != thi->timelisted_hwcounters.end(); j++)
					thi->MPIphases_total_counters[thi->currentIteration-1][thi->current_MPIphase][(*j).first] = (*j).second.lastreference;

				thi->current_MPIphase_time = e.Timestamp;
				thi->current_MPIphase++;

				/* RESET ALL COUNTERS ! */
				for (timelisted_hwcmap::iterator j = thi->timelisted_hwcounters.begin(); j != thi->timelisted_hwcounters.end(); j++)
					(*j).second.lastreference = 0;
			}


			for (vector<struct event_t>::iterator i = e.events.begin(); i != e.events.end(); i++)
				if ((*i).Type == iteration_event)
				{
					thi->fb.ResetBurst();
					thi->FirstIterationOcurred = ((*i).Value >= 1);
					thi->lastIterationTime = e.Timestamp;
					thi->currentIteration = (*i).Value;

					/* RESET ALL COUNTERS ! */
					for (timelisted_hwcmap::iterator j = thi->timelisted_hwcounters.begin(); j != thi->timelisted_hwcounters.end(); j++)
						(*j).second.lastreference = 0;

					has_reset = true;
					thi->current_MPIphase = 0;
					thi->current_MPIphase_time = e.Timestamp;
				}
		}
	}
	} /* PROCESS == 2 */
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
		     << "-split-out-samples/-no-split-out-samples" << endl
		     << "-exclude-big-ipc" << endl
		     << "-exclude-1st" << endl
		     << "-num-out-samples K" << endl
		     << "-choose-iteration I" << endl
		     << "-list-functions FILE" << endl
		     << "-list-functions-folded FILE" << endl
		     << "-max-iteration K" << endl 
		     << "-min-iteration K" << endl 
		     << "-sigma F" << endl 
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
		else if (strcmp ("-split-out-samples", argv[i]) == 0)
		{
			dump_splitted_counters = true;
			continue;
		}
		else if (strcmp ("-no-split-out-samples", argv[i]) == 0)
		{
			dump_splitted_counters = false;
			continue;
		}
		else if (strcmp ("-dump-counters", argv[i]) == 0)
		{
			i++;
			if (atoi(argv[i]) <= 0)
				cerr << "Invalid task number (must be between 1 and N)" << endl;
			else
				dump_counters_gnuplot_task = atoi(argv[i])-1;
			i++;
			if (atoi(argv[i]) <= 0)
				cerr << "Invalud thread number (must be between 1 and N)" << endl;
			else
				dump_counters_gnuplot_thread = atoi(argv[i])-1;
			dump_counters_gnuplot = true;
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
		else if (strcmp ("-num-out-samples", argv[i]) == 0)
		{
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid number of samples" << endl;
			else
				num_out_samples = atoi(argv[i]);
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
		else if (strcmp ("-out-iteration-event", argv[i]) == 0)
		{
			i++;
			if (atoi (argv[i]) == 0)
				cerr << "Invalid number of event" << endl;
			else
				out_iteration_event = atoi(argv[i]);
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
#if 0
		else if (strcmp ("-list-functions-folded", argv[i]) == 0)
		{
			i++;
			rutines_fold = ReadFile (argv[i]);
			continue;
		}
#endif
		else if (strcmp ("-list-functions", argv[i]) == 0)
		{
			i++;
			lfunctions_file = argv[i];
			/* rutines = ReadFile (argv[i]); */
		}
		else
			cerr << "unknown option " << argv[i] << endl;
	}

	cout << "Parsing file " << argv[argc-1] << " from iteration " << min_iteration << " to " << max_iteration << endl;
	cout << "Iteration event = " << iteration_event <<  " Out iteration event = " << out_iteration_event << endl;
	cout << "Target iteration " << chosen_iteration << endl;
	cout << "Sigma = " << sigma_times << endl;

	if (lfunctions_file.length() > 0)
		::ReadCallerLinesIntoList (lfunctions_file, string(argv[argc-1]));

	return argc-1;
}


void Process::dumpCounters_entireIteration (unsigned task, unsigned thread)
{
	ThreadInformation *thi = &(((ih.getTasksInformation())[task].getThreadsInformation())[thread]);

	/* HWC ! */
	for (timelisted_hwcmap::iterator k = thi->timelisted_hwcounters.begin(); k != thi->timelisted_hwcounters.end(); k++)
	{

#if defined(JUST_INSTRUCTIONS)
		if ((*k).first != PAPI_INSTRUCTIONS)
			continue;
#endif
		int N = num_out_samples; /* thi->MPIphases_outsamples[chosen_iteration-1][0]; */

		if (N > 0)
		{
			list<TimeValue> ltv = (*k).second.timevalue;
			unsigned long long last_counter_value;

			int npoints = ltv.size();
			double *points_x = (double*) malloc ((npoints+1)*sizeof(double));
			double *points_y = (double*) malloc ((npoints+1)*sizeof(double));
			if (points_x == NULL || points_y == NULL)
			{
				fprintf (stderr, "Failed to allocate points_x | points_y\n");
				exit (-1);
			}

			points_x[0] = points_y[0] = 0;

			double min_phase_time = (float) (thi->MPIphases[chosen_iteration-1][66]-thi->MPIphases[chosen_iteration-1][0])/(float) (thi->MPIphases[chosen_iteration-1][thi->numMPIs[0]-1]-thi->MPIphases[chosen_iteration-1][0]);
			double max_phase_time = (float) (thi->MPIphases[chosen_iteration-1][66+1]-thi->MPIphases[chosen_iteration-1][0])/(float) (thi->MPIphases[chosen_iteration-1][thi->numMPIs[0]-1]-thi->MPIphases[chosen_iteration-1][0]);

			int index = 1;

			list<TimeValue>::iterator iterador = ltv.begin();

			while (iterador != ltv.end())
			{
				cout << "MYPOINT2 " << (*iterador).normalized_time << " " << (*iterador).value << endl;
				if ((*iterador).normalized_time >= min_phase_time && (*iterador).normalized_time <= max_phase_time)
					cout << "MYPOINT3 " << (*iterador).normalized_time << " " << (*iterador).value << endl;
				iterador++;
			}

			iterador = ltv.begin();

			while (iterador != ltv.end())
			{
				int points_on_this_time = 1;
				points_x[index] = (*iterador).normalized_time;
				points_y[index] = (*iterador).value;

				iterador++;

				if (iterador != ltv.end())
					while ((*iterador).normalized_time == points_x[index])
					{
						points_y[index] += (*iterador).value;
						points_on_this_time++;
						iterador++;
						if (iterador == ltv.end())
					break;
					}
				points_y[index] = points_y[index]/points_on_this_time;

/*#if DUMP_COUNTERS*/
#if 1
#if defined(MPI_PHASES_TIME)
				if ((*k).first == PAPI_INSTRUCTIONS)
					cout << "MYPOINTS " << points_x[index] << " " << points_y[index] << endl;
#else
				if ((*k).first == PAPI_INSTRUCTIONS)
					cout << "MYPOINTS " << thi->MPIphases[chosen_iteration-1][0] + ((double)index/(double)N)*thi->MPIphases_length[chosen_iteration-1][0] << " " << points_y[index] << endl;
#endif

#endif
				// cout << "px["<<index<<"]="<< points_x[index]<<"; py["<<index<<"]="<<points_y[index]<<";" << endl;
				index++;
			}

			npoints = index;

			cout << "KRIGER (phase=" << 0 << ", inpoints=" << npoints << ", outpoints=" << N << ", hwc=" << (*k).first << ") for task " << task << " thread " << thread << endl;

			/* int N = num_out_samples; */
			last_counter_value = 0;
			unsigned long long time;
			bool put_zero = false;
			int skipped_values = 0;

			double * res_kri = (double*) malloc ((N+1)*sizeof(double));

			Kriger_Wrapper (npoints, points_x, points_y, N+1, res_kri, 0.0f, 1.0f);


			if (dump_counters_gnuplot)
			{
				if (dump_counters_gnuplot_task == task && dump_counters_gnuplot_thread == thread)
					cout << "KRIGER - COUNTER " << (*k).first << " TIME " << 0 << " VALUE " << 0 << endl;
			}
			else
			{
				DumpParaverLine (outfile, (*k).first + 600000000, 0, thi->timeChosenIteration, task+1, thread+1);
			}

			for (index = 1; index <= N; index++)
			{
#if !defined(MPI_PHASES_TIME)

# if defined(USE_MIN_TIME_ITERATION)
				time = thi->timeChosenIteration+((double)index/(double)N)*thi->minTimeIteration;
# elif defined(USE_MAX_TIME_ITERATION)
				time = thi->timeChosenIteration+((double)index/(double)N)*thi->maxTimeIteration;
# else
#  error "CHECK ME"
#endif

#else
				time = thi->MPIphases[chosen_iteration-1][0] + ((double)index/(double)N)*(thi->MPIphases[chosen_iteration-1][thi->numMPIs[0]-1]-(thi->MPIphases[chosen_iteration-1][0]));


#endif /* MPI_PHASES_TIME */

				if (dump_counters_gnuplot)
				{
					if (dump_counters_gnuplot_task == task && dump_counters_gnuplot_thread == thread)
					{
						if (res_kri[index] >= 0.0f)
						{
							if (last_counter_value < (unsigned long long) res_kri[index])
							{
								cout << "KRIGER - COUNTER " << (*k).first << " TIME " << ((double)index/(double)N) << " VALUE " << (unsigned long long) res_kri[index] - last_counter_value  << endl;
								last_counter_value = (unsigned long long) res_kri[index];
							}
						}
					}
				}
				else
				{
					if (res_kri[index] >= 0.0f)
					{
						if (last_counter_value < (unsigned long long) res_kri[index])
						{
							DumpParaverLine (outfile, (*k).first + 600000000, (unsigned long long) res_kri[index] - last_counter_value, time, task+1, thread+1);
							last_counter_value = (unsigned long long) res_kri[index];
							put_zero = false;
						}
						else
						{
							if (!put_zero)
								DumpParaverLine (outfile, (*k).first + 600000000, 0, time, task+1, thread+1);
							put_zero = true;
							skipped_values++;
						}
					}
				}
			}

			if (skipped_values > 0)
				cout << skipped_values << " skipped values" << endl;

			free (res_kri);

			if (!dump_counters_gnuplot)
			{
# if defined(USE_MIN_TIME_ITERATION)
				time = thi->timeChosenIteration+0.0f*thi->minTimeIteration;
				DumpParaverLine (outfile, out_iteration_event, 1, time, task+1, thread+1);
				time = thi->timeChosenIteration+1.0f*thi->minTimeIteration;
				DumpParaverLine (outfile, out_iteration_event, 0, time, task+1, thread+1);
# elif define(USE_MAX_TIME_ITERATION)
				time = thi->timeChosenIteration+0.0f*thi->maxTimeIteration;
				DumpParaverLine (outfile, out_iteration_event, 1, time, task+1, thread+1);
				time = thi->timeChosenIteration+1.0f*thi->maxTimeIteration;
				DumpParaverLine (outfile, out_iteration_event, 0, time, task+1, thread+1);
# else
#  error "Check me"
# endif
			}

			free (points_x);	
			free (points_y);
		}
	}

	for (unsigned index_phase = 0; index_phase < thi->numMPIs[0]-1; index_phase ++)
		DumpParaverLine (outfile, 100000, index_phase, thi->MPIphases[chosen_iteration-1][index_phase+1], task+1, thread+1);
	DumpParaverLine (outfile, 100000, 0, thi->MPIphases[chosen_iteration-1][thi->numMPIs[0]-1], task+1, thread+1);
}

void Process::dumpCounters_splitIteration (unsigned task, unsigned thread)
{
	bool dumped_stacks = false;
	unsigned last_sample_call = 0;

	if (task != 0 && thread != 0)
		return;

	ThreadInformation *thi = &(((ih.getTasksInformation())[task].getThreadsInformation())[thread]);

	/* HWC ! */
	for (timelisted_hwcmap::iterator k = thi->timelisted_hwcounters.begin(); k != thi->timelisted_hwcounters.end(); k++, dumped_stacks = true)
	{
		double last_counter_value = 0.0f;
		double last_time_value = 0.0f;

		for (unsigned index_phase = 0; index_phase < thi->numMPIs[0]; index_phase ++)
		{
			if (index_phase%2 == 1)
			{
				last_counter_value = 0.0f;
				last_time_value = 0.0f;

				int N = thi->MPIphases_outsamples[chosen_iteration-1][index_phase];

				if (N > 2)
				{
					list<TimeValue> ltv = (*k).second.timevalue;

					int npoints = ltv.size();
					double *points_x = (double*) malloc ((npoints+2)*sizeof(double));
					double *points_y = (double*) malloc ((npoints+2)*sizeof(double));
					if (points_x == NULL || points_y == NULL)
					{
						fprintf (stderr, "Failed to allocate points_x | points_y\n");
						exit (-1);
					}

					points_x[0] = points_y[0] = 0.0f;

					int index = 1;

					cout << "DEBUG range mintime=" << thi->MPIphases[chosen_iteration-1][index_phase] << " maxtime=" << thi->MPIphases[chosen_iteration-1][index_phase+1] << endl;
					cout << "DEBUG range min=" << thi->MPIphases_total_counters[chosen_iteration-1][index_phase][(*k).first] << " max=" << thi->MPIphases_total_counters[chosen_iteration-1][index_phase][(*k).first] << endl;
					unsigned delta_phase_time = thi->MPIphases[chosen_iteration-1][index_phase+1]-thi->MPIphases[chosen_iteration-1][index_phase];

					list<TimeValue>::iterator iterador = ltv.begin();
					while (iterador != ltv.end())
					{
						if ((*iterador).phase == index_phase)
						{
							double v;
							int iteration = (*iterador).iteration-1;

							points_x[index] = (*iterador).normal;
							v = normalize_value ((*iterador).value, 0, thi->MPIphases_total_counters[iteration][index_phase][(*k).first]);
							if (!isnan(v))
								points_y[index] = v;
							else
								points_y[index] = 0;

							if (points_x[index] == 1.0f && points_y[index] == 1.0f)
							{
								iterador++;
								continue;
							}

							unsigned long long time = thi->MPIphases[chosen_iteration-1][index_phase]+points_x[index]*delta_phase_time;

							/* HSG 
							   some rounding problems can appear when double*long, fix them */
							if (!dumped_stacks)
							{
								if (time < thi->MPIphases[chosen_iteration-1][index_phase])
									time = thi->MPIphases[chosen_iteration-1][index_phase];
								else if (time > thi->MPIphases[chosen_iteration-1][index_phase+1])
									time = thi->MPIphases[chosen_iteration-1][index_phase+1];

								if ((*iterador).sample_address != 0)
									DumpParaverLine (outfile, 34000000, (*iterador).sample_address, time, task+1, thread+1);

								if (task == 0 && thread == 0 && time >= 520000000 && time <= 540000000)
								{
									cout << "LAST_SAMPLE_CALL = " << last_sample_call << " CURRENT_SAMPLE_CALL = " << (*iterador).sample_call << " @ " << time << endl;
								}

								if ((*iterador).sample_call != 0)
								{
									if (last_sample_call == (*iterador).sample_call)
									{
										DumpParaverLine (outfile, 35000000, (*iterador).sample_call, time, task+1, thread+1);
									}
									else
									{
										last_sample_call = (*iterador).sample_call;
										DumpParaverLine (outfile, 35000000, 0, time, task+1, thread+1);
									}
								}
							}

#if 1
							if (task == 1 && thread == 0 /*&& (*k).first == PAPI_INSTRUCTIONS*/)
								cout << "INPOINTS2 raw index " << index << " X " << points_x[index] << " Y " <<  points_y[index] << " it " << (*iterador).iteration << " phase " << index_phase << " timestamp " << (*iterador).absolute << " " << (*k).first << endl;
#endif

							iterador++;
							index++;
						}
						else
							iterador++;
					}

					points_x[index] = points_y[index] = 1.0f;

					npoints = ++index;

					cout << "KRIGER (phase=" << index_phase << ", inpoints=" << npoints << ", outpoints=" << N << ", hwc=" << (*k).first << ") for task " << task << " thread " << thread << endl;

					/* int N = num_out_samples; */
					unsigned long long time;
					unsigned long long delta_time;
					bool put_zero = false;
					int skipped_values = 0;

					double * res_kri = (double*) malloc ((N+1)*sizeof(double));

					Kriger_Wrapper (npoints, points_x, points_y, N+1, res_kri, 0.0f, 1.0f);

					if (dump_counters_gnuplot)
					{
						if (dump_counters_gnuplot_task == task && dump_counters_gnuplot_thread == thread)
							cout << "KRIGER - COUNTER " << (*k).first << " TIME 0 VALUE 0 " << endl;
					}
					else
						DumpParaverLine (outfile, (*k).first + 600000000, 0, thi->timeChosenIteration, task+1, thread+1);

					int start_index = 0;
					while (start_index < N && res_kri[start_index] < last_counter_value)
					{
						start_index++;
						last_counter_value = res_kri[start_index];
						cout << "Skipping " << res_kri[start_index] << " at " << ((double) start_index / (double) N) << endl;
					}

#if 1
					if (task == 1 && thread == 0 /* && (*k).first == PAPI_INSTRUCTIONS */)
					{
						cout << "KRI-AC1 " << (*k).first << " phase " << index_phase << " " << ((double)0/(double)N) << " VALUE " << 0 << " phase_time " << ((double)0/(double)N) << endl;
						cout << "KRI-AC2 " << (*k).first << " phase " << index_phase << " " << ((double)0/(double)N) << " min " << 0 << " max " << thi->MPIphases_total_counters[chosen_iteration-1][index_phase][(*k).first] << " counter " << 0 << endl;

						cout << "KRI-AC3 " << (*k).first << " phase " << index_phase << " " << ((double)0/(double)N)  << " slope " << 0 << " deltacounter " << 0 << " deltatime " << 0 << endl;
					}
#endif

					delta_time = thi->MPIphases[chosen_iteration-1][index_phase+1]-thi->MPIphases[chosen_iteration-1][index_phase];
					last_time_value = 0.0f;

					for (index = start_index; index <= N; index++)
					{

						time = thi->MPIphases[chosen_iteration-1][index_phase]+((float)index/(float)N)*delta_time;

						/* HSG 
						   some rounding problems can appear when double*long, fix them */
						if (time < thi->MPIphases[chosen_iteration-1][index_phase])
							time = thi->MPIphases[chosen_iteration-1][index_phase];
						else if (time > thi->MPIphases[chosen_iteration-1][index_phase+1])
							time = thi->MPIphases[chosen_iteration-1][index_phase+1];

						if (dump_counters_gnuplot)
						{
							if (dump_counters_gnuplot_task == task && dump_counters_gnuplot_thread == thread)
							{
								if (res_kri[index] >= 0.0f)
								{
									if (last_counter_value < res_kri[index])
									{
										double delta_counter = res_kri[index] - last_counter_value;
										unsigned long long delta = delta_counter*thi->MPIphases_total_counters[chosen_iteration-1][index_phase][(*k).first];
										cout << "KRIGER - COUNTER " << (*k).first << " TIME " << ((double)index/(double)N) << " VALUE " << res_kri[index] - last_counter_value  << " ABSVALUE " << delta << endl;
										last_counter_value = res_kri[index];
									}
								}
							}
						}
						else
						{
							if (res_kri[index] >= 0.0f)
							{
								if (last_counter_value < res_kri[index])
								{
									double delta_counter = res_kri[index] - last_counter_value;
									unsigned long long delta = delta_counter*thi->MPIphases_total_counters[chosen_iteration-1][index_phase][(*k).first];
									unsigned long long counter = res_kri[index]*thi->MPIphases_total_counters[chosen_iteration-1][index_phase][(*k).first];

									DumpParaverLine (outfile, (*k).first + 600000000, delta, time, task+1, thread+1);

									if (task == 0 && thread == 0)
										cout << "type = " <<  (*k).first + 600000000 << " delta= " << delta << " delta_counter= " << delta_counter << " total_value= " << thi->MPIphases_total_counters[chosen_iteration-1][index_phase][(*k).first] << endl;

									// DumpParaverLine (outfile, (*k).first + 800000000, counter , time, task+1, thread+1);

#if 1
									if (task == 1 && thread == 0 /*&& (*k).first == PAPI_INSTRUCTIONS*/)
									{
										cout << "KRI-AC1 " << (*k).first << " phase " << index_phase << " " << ((double)index/(double)N) << " VALUE " << res_kri[index] << " phase_time " << ((double)index/(double)N) << endl;
										cout << "KRI-AC2 " << (*k).first << " phase " << index_phase << " " << ((double)index/(double)N) << " min " << 0 << " max " << thi->MPIphases_total_counters[chosen_iteration-1][index_phase][(*k).first] << " counter " << counter << endl;

									}
#endif

									last_counter_value = res_kri[index];
									last_time_value = ((double)index/(double)N);

									put_zero = false;
								}
								else
								{
									if (!put_zero)
									{
										DumpParaverLine (outfile, (*k).first + 600000000, 0, time, task+1, thread+1);
										// DumpParaverLine (outfile, (*k).first + 800000000, 0, time, task+1, thread+1);
									}
									put_zero = true;
									skipped_values++;
								}
								if (task == 1 && thread == 0)
									cout << "KRI-AC3 " << (*k).first << " phase " << index_phase << " " << ((double)index/(double)N)  << " slope " << (res_kri[index]-res_kri[index-1])/(1.0f/(double)N) << " deltacounter " << (res_kri[index]-res_kri[index-1])/((double)N) << " deltatime " << 1.0f/N << endl;
							}
						}
					}

					if (skipped_values > 0)
						cout << skipped_values << " skipped values" << endl;

					free (res_kri);

					free (points_x);	
					free (points_y);
				}
			}
			else /* index_phase%2 == 1 */
			{
				unsigned long long time = thi->MPIphases[chosen_iteration-1][index_phase+1];
				DumpParaverLine (outfile, (*k).first + 600000000, 0, time, task+1, thread+1);
				// DumpParaverLine (outfile, (*k).first + 800000000, 0, time, task+1, thread+1);
			}
			DumpParaverLine (outfile, 100000, index_phase, thi->MPIphases[chosen_iteration-1][index_phase+1], task+1, thread+1);
		}
		DumpParaverLine (outfile, 100000, 0, thi->MPIphases[chosen_iteration-1][thi->numMPIs[0]-1], task+1, thread+1);
		DumpParaverLine (outfile, 34000000, 0, thi->MPIphases[chosen_iteration-1][thi->numMPIs[0]-1], task+1, thread+1);
		DumpParaverLine (outfile, 35000000, 0, thi->MPIphases[chosen_iteration-1][thi->numMPIs[0]-1], task+1, thread+1);
	}
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
	int res;

	res = ProcessParameters (argc, argv);

	Process *p = new Process (argv[res], true);
	vector<ParaverTraceApplication *> va = p->get_applications();
	if (va.size() != 1)
	{
		cerr << "ERROR Cannot parse traces with more than one application" << endl;
		return -1;
	}

	outfile.open (argv[res],ios::out|ios::app);

	p->listids = rutines;
/*
	p->listids.push_back (3);
	p->listids.push_back (4);
	p->listids.push_back (5);
	p->listids.push_back (6);
	p->listids.push_back (7);
*/

	for (unsigned int i = 0; i < va.size(); i++)
	{
		vector<ParaverTraceTask *> vt = va[i]->get_tasks();
		p->ih.AllocateTasks (vt.size());
		TaskInformation *ti = p->ih.getTasksInformation();
		for (unsigned int j = 0; j < vt.size(); j++)
			ti[j].AllocateThreads(vt[j]->get_threads().size());
	}

	p->PROCESS = 1;
	p->parseBody();

	if (exclude_big_ipc || check_time_outlier || check_counter_outlier)
	{
		for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
		{
			TaskInformation *ti = p->ih.getTasksInformation();
			for (unsigned j = 0; j < ti[i].getNumThreads(); j++) 
			{
				ThreadInformation *thi = ti[i].getThreadsInformation();
				unsigned long long tmp_counter = 0, tmp_time = 0;
				unsigned long long mitja_counter, mitja_time;
	
				for (unsigned int k = min_iteration-1; k < MIN(thi[j].total_counters.size(), max_iteration); k++)
				{
					tmp_counter += thi[j].total_counters[k][PAPI_INSTRUCTIONS];
					tmp_time    += thi[j].maxTimes[k];
				}
				mitja_counter = tmp_counter / (MIN(thi[j].total_counters.size(), max_iteration) - min_iteration - 1); /* thi[j].total_counters.size(); */
				mitja_time    = tmp_time    / (MIN(thi[j].total_counters.size(), max_iteration) - min_iteration - 1); /* thi[j].total_counters.size(); */
	
				tmp_counter = tmp_time = 0;
				for (unsigned int k = 0; k < thi[j].total_counters.size(); k++)
				{
					tmp_counter += (thi[j].total_counters[k][PAPI_INSTRUCTIONS]-mitja_counter)*(thi[j].total_counters[k][PAPI_INSTRUCTIONS]-mitja_counter);
					tmp_time    += (thi[j].maxTimes[k]-mitja_time)*(thi[j].maxTimes[k]-mitja_time);
				}
				if (thi[j].total_counters.size()>1)
				{
					thi[j].sigma_counter = sqrtf ((float) (tmp_counter)/(float) (thi[j].total_counters.size()-1));
					thi[j].sigma_time    = sqrtf ((float) (tmp_time)   /(float) (thi[j].total_counters.size()-1));
				}
				else
					thi[j].sigma_time = thi[j].sigma_counter = 0.0f;
	
				for (unsigned int k = 0; k < min_iteration-1; k++)
					thi[j].excluded_iteration.push_back (true);

				for (unsigned int k = min_iteration-1; k < thi[j].total_counters.size(); k++)
				{
					bool excluded = (k==0)?exclude_1st:false;
					if (exclude_big_ipc)
						excluded |= thi[j].greatIPC[k];
					if (check_time_outlier)
						excluded |= fabs ((float)(thi[j].maxTimes[k])-(float)(mitja_time)) > (sigma_times*thi[j].sigma_time);
					if (check_counter_outlier)
						excluded |= fabs ((float)(thi[j].total_counters[k][PAPI_INSTRUCTIONS])-(float)(mitja_counter)) > (sigma_times*thi[j].sigma_counter);

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
			thi[thread].maxTimeIteration = 0;
			thi[thread].minTimeIteration = 0xffffffffffffffffULL;

			/* Choose on which iteration will dump the data (0, means first) */
			thi[thread].minTimeIteration = thi[thread].maxTimes[chosen_iteration-1];
			thi[thread].maxTimeIteration = thi[thread].minTimes[chosen_iteration-1];

			for (unsigned i = 0; i < MIN(thi[thread].maxTimes.size(), max_iteration); i++)
			{
				if (!thi[thread].excluded_iteration[i])
				{
#if defined(USE_MAX_TIME_ITERATION)
					thi[thread].normalization_factor.push_back (((float)thi[thread].maxTimeIteration/(float)thi[thread].maxTimes[i]));
#elif defined(USE_MIN_TIME_ITERATION)
					thi[thread].normalization_factor.push_back (((float)thi[thread].minTimeIteration/(float)thi[thread].minTimes[i]));
#else
# error "Define USE_MAX_TIME_ITERATION or USE_MIN_TIME_ITERATION"
#endif
				}
				else
					thi[thread].normalization_factor.push_back (1.0f);
			}
		}
	}

	/*** Check if all the iterations have the same number of MPI phases ***/
	for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
		for (unsigned j = 0; j < p->ih.getTasksInformation()[i].getNumThreads(); j++)
		{
			ThreadInformation *thi = p->ih.getTasksInformation()[i].getThreadsInformation();

			if (thi[j].numMPIs.size() > 0)
			{
				cout << "TASK " << i << " THREAD " << j << " Number of found phases : " << thi[j].numMPIs[0] << flush;

				bool equal = true;
				for (unsigned k = 0; k < thi[j].numMPIs.size(); k++)
					equal = equal && thi[j].numMPIs[k] == thi[j].numMPIs[0];

				if (!equal)
				{
					cerr << "The number of MPI phases is not consistent across iterations" << endl;
					exit (-1);
				}
				else
					cout << " Consistent across iterations" << endl;
			}
		}

	/*** Compute diff times for every MPI phases ***/
	for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
		for (unsigned j = 0; j < p->ih.getTasksInformation()[i].getNumThreads(); j++)
		{
			ThreadInformation *thi = p->ih.getTasksInformation()[i].getThreadsInformation();
			for (unsigned k = 0; k < thi[j].MPIphases.size(); k++)
			{
				vector<unsigned long long> v;
				thi[j].MPIphases_length.push_back(v);

				for (unsigned l = 0; l < (thi[j].MPIphases)[k].size()-1; l++)
				{
					unsigned long long diff = ((thi[j].MPIphases)[k])[l+1]-((thi[j].MPIphases)[k])[l];
					((thi[j].MPIphases_length)[k]).push_back (diff);
				}
			}
		}

	/*** Compute relative factor times for every MPI phases ***/
	for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
		for (unsigned j = 0; j < p->ih.getTasksInformation()[i].getNumThreads(); j++)
		{
			ThreadInformation *thi = p->ih.getTasksInformation()[i].getThreadsInformation();

			unsigned LAST = (thi[j].MPIphases_length)[chosen_iteration-1].size();
			unsigned long long total_iteration_time = ((thi[j].MPIphases)[chosen_iteration-1])[LAST] - ((thi[j].MPIphases)[chosen_iteration-1])[0];

			for (unsigned k = 0; k < thi[j].MPIphases.size(); k++)
			{
				thi[j].MPIphases_factor.push_back (* (new vector<float>) );
				thi[j].MPIphases_outsamples.push_back ( * (new vector<unsigned long long>) );

				thi[j].MPIphases_total_counters.push_back( * (new vector<hwcmap>) );

				for (unsigned l = 0; l < (thi[j].MPIphases_length)[k].size(); l++)
				{
					float factor = (float)((thi[j].MPIphases_length)[chosen_iteration-1])[l]/(float)((thi[j].MPIphases_length)[k])[l];
					((thi[j].MPIphases_factor)[k]).push_back (factor);

					float length_factor = (float)((thi[j].MPIphases_length)[chosen_iteration-1])[l]/(float)total_iteration_time;
					float mul = length_factor * num_out_samples;

					((thi[j].MPIphases_outsamples)[k]).push_back ( lround(mul+2) );

					thi[j].MPIphases_total_counters[k].push_back ( *(new hwcmap) );
				}
			}
		}

	/*** Create timelisted vector counters ***/
	for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
		for (unsigned j = 0; j < p->ih.getTasksInformation()[i].getNumThreads(); j++)
		{
			ThreadInformation *thi = p->ih.getTasksInformation()[i].getThreadsInformation();
			thi[j].FirstIterationOcurred = false;
			thi[j].currentIteration = 0;
		}


	p->PROCESS = 2;
	p->parseBody();


#if 0
	for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
	{
		cout << " i = " << i << endl;
		ThreadInformation *thi = ti[i].getThreadsInformation();
		for (unsigned j = 0; j < ti[i].getNumThreads(); j++) 
		{
			cout << " j = " << j << endl;
			for (unsigned k = 0; k < thi[j].numMPIs[0]; k++)
			{
				cout << " k = " << k << endl;
				for (timelisted_hwcmap::iterator l = thi[j].timelisted_hwcounters.begin(); l != thi[j].timelisted_hwcounters.end(); l++)
				{
					cout << "HIPERDEBUG " << i << "/" << j << ":" << " phase=" << k << " counter " << (*l).first << " range min=" <<
					  thi[j].MPIphases_total_counters[chosen_iteration-1][k][(*l).first] <<
					  " rangemax=" << thi[j].MPIphases_total_counters[chosen_iteration-1][k][(*l).first] <<
					  endl;
				}
			}
		}
	}
#endif
	
// #define EXCLUSIO_OUTLIERS_LOCALS
#warning "EXCLUSIO_OUTLIERS_LOCALS no esta definit, no cal?"
#if defined(EXCLUSIO_OUTLIERS_LOCALS)

#if !defined(MONO_TASK_THREAD)
	for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
#endif
	{
		TaskInformation *ti = p->ih.getTasksInformation();
#if defined(MONO_TASK_THREAD)
		int j = SELECTED_THREAD-1;
		int i = SELECTED_TASK-1;
#else
		for (unsigned j = 0; j < ti[i].getNumThreads(); j++) 
#endif
		{
			ThreadInformation *thi = ti[i].getThreadsInformation();

			for (timelisted_hwcmap::iterator k = thi[j].timelisted_hwcounters.begin(); k != thi[j].timelisted_hwcounters.end(); k++)
			{
#if defined(JUST_INSTRUCTIONS)
				if ((*k).first != PAPI_INSTRUCTIONS)
					continue;
#endif
				list<TimeValue> ltv = (*k).second.timevalue;
				float longitud_particions = 1.0f / (float) numpartitions;
				float next_partition_time = longitud_particions;

				(*k).second.timevalue.clear();

				list<TimeValue>::iterator it = ltv.begin();
				while (it != ltv.end())
				{
					vector<TimeValue> mostres;
					float mitjana_comptadors = 0.0f;
					while ( (*it).normalized_time < next_partition_time && it != ltv.end())
					{
						mitjana_comptadors += (*it).value; /* o real_value */
						mostres.push_back (*it);
						it++;
					}

					if (mostres.size() > 0)
						mitjana_comptadors /= mostres.size();
					float tmp = 0.0f;
					for (unsigned index = 0; index < mostres.size(); index++)
						tmp += (mostres[index].value-mitjana_comptadors)*(mostres[index].value-mitjana_comptadors);
					if (mostres.size() > 1)
						tmp /= (mostres.size()-1);
					tmp = sqrtf (tmp);

					cout << "TYPE 42000050 VALUE " << (unsigned long long) mitjana_comptadors << " ACCUMULATED " << (unsigned long long) mitjana_comptadors << " TIME " << next_partition_time-longitud_particions/2 << " ITERATION 5004" << endl;

					for (unsigned index = 0; index < mostres.size(); index++)
					{
						if (fabs(mostres[index].value-mitjana_comptadors) < sigma_times*tmp) /*sigma*/
							(*k).second.timevalue.push_back (mostres[index]);
						else
							cout << "TYPE 42000050 VALUE " << mostres[index].realvalue << " ACCUMULATED " << mostres[index].value << " TIME " << mostres[index].normalized_time << " ITERATION 5003" << endl;
					}

					next_partition_time += longitud_particions;
				}

				// cout << "POST SIZE " << (*k).second.timevalue.size() << endl;
			}
		}
	}
#endif /* OUTLIERS SIGMA2 particions */

	// outfile.open (argv[res],ios::out|ios::app);


	int i_min = 0;
	int i_max = p->ih.getNumTasks();

	if (dump_counters_gnuplot)
	{
		i_min = dump_counters_gnuplot_task;
		i_max = i_min+1;
	}

	for (int i = i_min; i <  i_max; i++)
	{
		TaskInformation *ti = p->ih.getTasksInformation();

		int j_min = 0;
		int j_max = ti[i].getNumThreads();

		if (dump_counters_gnuplot)
		{
			j_min = dump_counters_gnuplot_thread;
			j_max = j_min+1;
		}

		for (int j = j_min; j < j_max; j++) 
		{
			ThreadInformation *thi = ti[i].getThreadsInformation();


			/* Sort elements by minor time */
			list<Function> lf = thi[j].fb.getFunctionBursts();
			lf.sort();

			/* Merge elements now where IDs can be merged */
			list<Function>::iterator it;
			list<Function> result;
			unsigned long long lastID = 0;

#define MERGE_THEM
#if defined(MERGE_THEM)
			it = lf.begin();
			while (it != lf.end())
			{
				lastID = (*it).getID();
				Function F(lastID);
				F.setMaxTime ((*it).getMaxTime());
				F.setMinTime ((*it).getMinTime());

				while ((*(++it)).getID() == lastID)
				{
					if (it == lf.end())
						break;
					F.setMaxTime ((*it).getMaxTime());
					F.setMinTime ((*it).getMinTime());
				}
				result.push_back(F);
			}
#else
			result = lf;
#endif

			list<Function> final;
			list<Function>::iterator it_ahead = result.begin();
			it_ahead++;

			cout << "Task " << i+1 << " Thread " << j+1 << endl;
			for (it = result.begin(); it_ahead != result.end(); it_ahead++, it++)
			{
#if 0
				(*it).dump();
				(*it_ahead).dump();
#endif

				if ((*it_ahead).getMinTime() < (*it).getMaxTime())
				{
					unsigned long long tmp;
					Function f_ahead = *it_ahead;
					Function f = *it;

					cout << "MIXED REGION -- Correcting ..." << endl;
					tmp = f.getMaxTime();
					f.forceMaxTime (f_ahead.getMinTime());
					f_ahead.forceMinTime (tmp);
					f.dump();
					f_ahead.dump();
					cout << "MIXED REGION -- End of correction" << endl;

					*it = f;
					*it_ahead = f_ahead;
				}
			}

			for (it = result.begin(); it != result.end(); it++)
			{
				/* (*it).dump(); */
				if ((*it).getMinTime() < thi[j].minTimeIteration+thi[j].timeChosenIteration)
				{
					p->DumpParaverLine (outfile, 35000000, (*it).getID(), (*it).getMinTime(), i+1, j+1);
				}
				else
					break;

				if ((*it).getMaxTime() < thi[j].minTimeIteration+thi[j].timeChosenIteration)
				{
					p->DumpParaverLine (outfile, 35000000, 0, (*it).getMaxTime(), i+1, j+1);
				}
				else
				{
					p->DumpParaverLine (outfile, 35000000, 0, thi[j].minTimeIteration+thi[j].timeChosenIteration, i+1, j+1);
					break;
				}
			}

			if (dump_splitted_counters)
				p->dumpCounters_splitIteration (i, j);
			else
				p->dumpCounters_entireIteration (i, j);

		}
	}

	outfile.close();

	return 0;
}
