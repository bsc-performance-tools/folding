
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

#include "globals.H"
#include "process.H"

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

void ReadCallerLinesIntoList (string file, string tracefile);

namespace domain {

list<unsigned long long> LlistaIDcallerlines;
list<unsigned long long> LlistaIDcaller;

unsigned int out_iteration_event = 1234567;

bool dump_counters_gnuplot = false;
unsigned dump_counters_gnuplot_thread, dump_counters_gnuplot_task;

bool use_phase_count = true;

int num_out_samples = 1000;
list<unsigned long long> rutines_fold;
list<unsigned long long> rutines;

fstream outfile;

using namespace std;


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
	bool in_mpi = false, in_uf = false, in_phase_change = false;
	unsigned task = e.ObjectID.task - 1;
	unsigned thread = e.ObjectID.thread - 1;

	if (task >= ih.getNumTasks())
		return;

	TaskInformation *ti = ih.getTasksInformation();
	if (thread >= ti[task].getNumThreads())
		return;

	ThreadInformation *thi = &((ti[task].getThreadsInformation())[thread]);

	bool has_reset = false;

	/* Check consistency */
	if (task < ih.getNumTasks())
	{
		unsigned long long type;
		unsigned long long value;

		if (thread < ti[task].getNumThreads())
		{
			/* Are we in a user function event or in a MPI event? */
			in_mpi = false;
			in_uf = false;
			bool has_sample = false;
			for (vector<struct event_t>::iterator i = e.events.begin(); i != e.events.end(); i++)
			{
				in_mpi = in_mpi || ((*i).Type == 50000001 || (*i).Type == 50000002 || (*i).Type == 50000003);
				in_uf = in_uf || (*i).Type == 60000019;
				in_phase_change = in_phase_change || (*i).Type == phase_separator;
				has_sample = has_sample || ((*i).Type >= 30000000 && (*i).Type <= 30000100);
			}

			/* Some MPIs are splitted into several lines. Keep track of latest MPI times */
			if (in_mpi)
				thi->lastMPItime = e.Timestamp;

			if (!in_mpi && thi->lastMPItime == e.Timestamp)
				in_mpi = true;

#if 0
			unsigned top_callstack_sample;
			unsigned top_callstack_sample_level;
			unsigned top_callstack_call;

			/* Get the top of the known stack -- known as user provide */
			if (has_sample)
			{
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

#if 0
						tv.sample_address =  (has_sample)?top_callstack_sample:0;
						tv.sample_call = (has_sample)?top_callstack_call:0;
#else
						tv.sample_address =  0;
						tv.sample_call = 0;
#endif

						tv.time = e.Timestamp - thi->lastIterationTime;
						tv.normalized_time = (float)(e.Timestamp - thi->current_phase_time)/(float)(thi->phases_length[thi->currentIteration-1][thi->current_phase]);
						tv.absolute = e.Timestamp;

#if 0
						tv.phase = thi->current_phase;
#else
						tv.phase = (use_phase_count)?thi->current_phase:thi->phases_map[thi->current_phase-1];
#endif

						if (!has_reset)
						{
							tv.value = thi->timelisted_hwcounters[type].lastreference + value;
							tv.realvalue = value;
						}
						else
							tv.value = tv.realvalue = 0;
						tv.iteration = thi->currentIteration;

#if 0
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
#endif

						thi->timelisted_hwcounters[type].timevalue.push_back (tv);

						thi->timelisted_hwcounters[type].lastreference = tv.value;
					}
				}
				else if (thi->FirstIterationOcurred && (type >= PAPI_MIN_VAL && type <= PAPI_MAX_VAL) && thi->currentIteration <= max_iteration && thi->currentIteration >= min_iteration /* && in_mpi && in_uf */ )
				{
					if (!thi->excluded_iteration[thi->currentIteration-1])
						thi->timelisted_hwcounters[type].lastreference += value;
				}
			}

			if (thi->FirstIterationOcurred && in_phase_change && thi->currentIteration <= max_iteration && thi->currentIteration >= min_iteration)
			{
				for (timelisted_hwcmap::iterator j = thi->timelisted_hwcounters.begin(); j != thi->timelisted_hwcounters.end(); j++)
					thi->phases_total_counters[thi->currentIteration-1][thi->current_phase][(*j).first] = (*j).second.lastreference;

				thi->current_phase_time = e.Timestamp;
				thi->current_phase++;

				/* RESET ALL COUNTERS ! */
				for (timelisted_hwcmap::iterator j = thi->timelisted_hwcounters.begin(); j != thi->timelisted_hwcounters.end(); j++)
					(*j).second.lastreference = 0;
			}

			for (vector<struct event_t>::iterator i = e.events.begin(); i != e.events.end(); i++)
				if ((*i).Type == iteration_event)
				{
					// thi->fb.ResetBurst();
					thi->FirstIterationOcurred = ((*i).Value >= 1);
					thi->lastIterationTime = e.Timestamp;
					thi->currentIteration = (*i).Value;

					/* RESET ALL COUNTERS ! */
					for (timelisted_hwcmap::iterator j = thi->timelisted_hwcounters.begin(); j != thi->timelisted_hwcounters.end(); j++)
						(*j).second.lastreference = 0;

					has_reset = true;
					thi->current_phase = 1; /* phase 0 is a fake start! */
					thi->current_phase_time = e.Timestamp;
				}
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
		     << "-num-out-samples K" << endl
		     << "-choose-iteration I" << endl
		     << "-list-functions FILE" << endl
		     << "-list-functions-folded FILE" << endl
		     << "-max-iteration K" << endl 
		     << "-min-iteration K" << endl
		     << "-use-phase-value / -use-phase-count "<< endl
         << "-dump-counters TASK(1..N) THREAD(1..N)" << endl;
		exit (-1);
	}

	for (int i = 1; i < argc-1; i++)
	{
		if (strcmp ("-dump-counters", argv[i]) == 0)
		{
			i++;
			if (atoi(argv[i]) <= 0)
				cerr << "Invalid task number (must be between 1 and N)" << endl;
			else
				dump_counters_gnuplot_task = atoi(argv[i])-1;
			i++;
			if (atoi(argv[i]) <= 0)
				cerr << "Invalid thread number (must be between 1 and N)" << endl;
			else
				dump_counters_gnuplot_thread = atoi(argv[i])-1;
			dump_counters_gnuplot = true;
			continue;
		}
		else if (strcmp ("-use-phase-value", argv[i]) == 0)
		{
			use_phase_count = false;
		}
		else if (strcmp ("-use-phase-count", argv[i]) == 0)
		{
			use_phase_count = true;
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

	if (lfunctions_file.length() > 0)
		::ReadCallerLinesIntoList (lfunctions_file, string(argv[argc-1]));

	return argc-1;
}

void Process::dumpCounters_splitIteration (unsigned task, unsigned thread)
{
	unsigned long long match_phase;
	bool dumped_stacks = false;
	unsigned last_sample_call = 0;

	ThreadInformation *thi = &(((ih.getTasksInformation())[task].getThreadsInformation())[thread]);

	/* HWC ! */
	for (timelisted_hwcmap::iterator k = thi->timelisted_hwcounters.begin(); k != thi->timelisted_hwcounters.end(); k++, dumped_stacks = true)
	{
		double last_counter_value = 0.0f;
		double last_time_value = 0.0f;

#if 1	
		unsigned long long total = 0;
		for (unsigned index_phase = 1; index_phase < thi->max_numphase; index_phase++)
		{
			int translated = thi->phases_map[index_phase-1];
			cout << "DEBUG index phase = " << index_phase << " translated " << translated << " counter up to " << thi->phases_total_counters[chosen_iteration-1][index_phase][(*k).first] << endl;
			total +=  thi->phases_total_counters[chosen_iteration-1][index_phase][(*k).first];
		}
		cout << "DEBUG for a total of " << total << endl;
#endif

		for (unsigned index_phase = 1; index_phase < thi->max_numphase; index_phase++)
		{
			/* if (index_phase%2 == 1) */
			{
				match_phase = (use_phase_count)?index_phase:thi->phases_map[index_phase-1];
				last_counter_value = 0.0f;
				last_time_value = 0.0f;

				if (!use_phase_count)
					if (match_phase == 0)
						continue;

				int N = thi->phases_outsamples[chosen_iteration-1][index_phase];

				cout << "ANALYZING PHASE " << match_phase <<  " for N=" << N << " subpartitions" << endl;

				if (N > 1)
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

					cout << "DEBUG range mintime=" << thi->phases[chosen_iteration-1][index_phase] << " maxtime=" << thi->phases[chosen_iteration-1][index_phase+1] << endl;
					cout << "chosen_iteration="<<chosen_iteration<<" index_phase="<<index_phase<<endl;

					// TST cout << "DEBUG range total counter=" << thi->phases_total_counters[chosen_iteration-1][match_phase+1][(*k).first] << endl;
					cout << "DEBUG range total counter=" << thi->phases_total_counters[chosen_iteration-1][index_phase][(*k).first] << endl;
					unsigned delta_phase_time = thi->phases[chosen_iteration-1][index_phase+1]-thi->phases[chosen_iteration-1][index_phase];

					list<TimeValue>::iterator iterador = ltv.begin();
					while (iterador != ltv.end())
					{
#if 0
						cout << "TV @ " << (*iterador).time << " PHASE " << (*iterador).phase << endl;
#endif

						if ((*iterador).phase == match_phase)
						//if ((*iterador).phase == match_phase && ((*iterador).iteration <= 100 && (*iterador).iteration >= 2))
						{
							double v;
							int iteration = (*iterador).iteration-1;

							points_x[index] = (*iterador).normalized_time;
							v = normalize_value ((*iterador).value, 0, thi->phases_total_counters[iteration][index_phase][(*k).first]);
							// TST v = normalize_value ((*iterador).value, 0, thi->phases_total_counters[iteration][match_phase+1][(*k).first]);
							if (!isnan(v))
								points_y[index] = v;
							else
								points_y[index] = 0;

							if (points_x[index] == 1.0f && points_y[index] == 1.0f)
							{
								iterador++;
								continue;
							}

							unsigned long long time = thi->phases[chosen_iteration-1][index_phase]+points_x[index]*delta_phase_time;

							/* HSG 
							   some rounding problems can appear when double*long, fix them */
							if (!dumped_stacks)
							{
								if (time < thi->phases[chosen_iteration-1][index_phase])
									time = thi->phases[chosen_iteration-1][index_phase];
								else if (time > thi->phases[chosen_iteration-1][index_phase+1])
									time = thi->phases[chosen_iteration-1][index_phase+1];

								if ((*iterador).sample_address != 0)
									DumpParaverLine (outfile, 34000000, (*iterador).sample_address, time, task+1, thread+1);

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
							if (task == 0 && thread == 0 /*&& (*k).first == PAPI_INSTRUCTIONS*/)
								cout << "INPOINTS2 raw index " << index << " X " << points_x[index] << " Y " <<  points_y[index] << " it " << (*iterador).iteration << " phase " << index_phase << " timestamp " << (*iterador).absolute << " " << (*k).first << " MATCH_PHASE " << match_phase << endl;
#endif

							iterador++;
							index++;
						}
						else
							iterador++;
					}

					points_x[index] = points_y[index] = 1.0f;

					npoints = ++index;

					cout << "KRIGER (phase=" << index_phase << ", match_phase=" << match_phase << ", inpoints=" << npoints << ", outpoints=" << N << ", hwc=" << (*k).first << ") for task " << task << " thread " << thread << endl;

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
					if (task == 0 && thread == 0 /* && (*k).first == PAPI_INSTRUCTIONS */)
					{
						cout << "KRI-AC1 " << (*k).first << " phase " << index_phase << " " << ((double)0/(double)N) << " VALUE " << 0 << " phase_time " << ((double)0/(double)N) << endl;
						cout << "KRI-AC2 " << (*k).first << " phase " << index_phase << " " << ((double)0/(double)N) << " min " << 0 << " max " << thi->phases_total_counters[chosen_iteration-1][index_phase][(*k).first] << " counter " << 0 << endl;

						cout << "KRI-AC3 " << (*k).first << " phase " << index_phase << " " << ((double)0/(double)N)  << " slope " << 0 << " deltacounter " << 0 << " deltatime " << 0 << endl;
					}
#endif

					delta_time = thi->phases[chosen_iteration-1][index_phase+1]-thi->phases[chosen_iteration-1][index_phase];
					last_time_value = 0.0f;

					for (index = start_index; index <= N; index++)
					{

						time = thi->phases[chosen_iteration-1][index_phase]+((float)index/(float)N)*delta_time;

						/* HSG 
						   some rounding problems can appear when double*long, fix them */
						if (time < thi->phases[chosen_iteration-1][index_phase])
							time = thi->phases[chosen_iteration-1][index_phase];
						else if (time > thi->phases[chosen_iteration-1][index_phase+1])
							time = thi->phases[chosen_iteration-1][index_phase+1];

						if (dump_counters_gnuplot)
						{
							if (dump_counters_gnuplot_task == task && dump_counters_gnuplot_thread == thread)
							{
								if (res_kri[index] >= 0.0f)
								{
									if (last_counter_value < res_kri[index])
									{
										double delta_counter = res_kri[index] - last_counter_value;
										unsigned long long delta = delta_counter*thi->phases_total_counters[chosen_iteration-1][index_phase][(*k).first];
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
									unsigned long long delta = delta_counter*thi->phases_total_counters[chosen_iteration-1][index_phase][(*k).first];
									unsigned long long counter = res_kri[index]*thi->phases_total_counters[chosen_iteration-1][index_phase][(*k).first];

									DumpParaverLine (outfile, (*k).first + 600000000, delta, time, task+1, thread+1);

									if (task == 0 && thread == 0)
										cout << "type = " <<  (*k).first + 600000000 << " delta= " << delta << " delta_counter= " << delta_counter << " total_value= " << thi->phases_total_counters[chosen_iteration-1][index_phase][(*k).first] << endl;

									// DumpParaverLine (outfile, (*k).first + 800000000, counter , time, task+1, thread+1);

#if 1
									if (task == 0 && thread == 0 /*&& (*k).first == PAPI_INSTRUCTIONS*/)
									{
										cout << "KRI-AC1 " << (*k).first << " phase " << index_phase << " " << ((double)index/(double)N) << " VALUE " << res_kri[index] << " phase_time " << ((double)index/(double)N) << endl;
										cout << "KRI-AC2 " << (*k).first << " phase " << index_phase << " " << ((double)index/(double)N) << " min " << 0 << " max " << thi->phases_total_counters[chosen_iteration-1][index_phase][(*k).first] << " counter " << counter << endl;

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
								if (task == 0 && thread == 0)
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
#if 0
			else /* index_phase%2 == 1 */
			{
				unsigned long long time = thi->phases[chosen_iteration-1][index_phase+1];
				DumpParaverLine (outfile, (*k).first + 600000000, 0, time, task+1, thread+1);
				// DumpParaverLine (outfile, (*k).first + 800000000, 0, time, task+1, thread+1);
			}
#endif
			DumpParaverLine (outfile, 100000, index_phase, thi->phases[chosen_iteration-1][index_phase+1], task+1, thread+1);
		}
		DumpParaverLine (outfile, 100000, 0, thi->phases[chosen_iteration-1][thi->max_numphase-1], task+1, thread+1);
		DumpParaverLine (outfile, 34000000, 0, thi->phases[chosen_iteration-1][thi->max_numphase-1], task+1, thread+1);
		DumpParaverLine (outfile, 35000000, 0, thi->phases[chosen_iteration-1][thi->max_numphase-1], task+1, thread+1);
	}
}

void ReadCallerLinesIntoList (string file, string tracefile)
{
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
	UIParaverTraceConfig *pcf = new UIParaverTraceConfig (pcffile);
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

	string *tracefile = new string(argv[res]);
	string traceprefix = tracefile->substr (0, tracefile->rfind ("."));
	p->ReadInformation (traceprefix);

  /*** Compute relative factor times for every MPI phases ***/
  for (unsigned i = 0; i < p->ih.getNumTasks(); i++)
    for (unsigned j = 0; j < p->ih.getTasksInformation()[i].getNumThreads(); j++)
    {
      ThreadInformation *thi = p->ih.getTasksInformation()[i].getThreadsInformation();

      unsigned long long total_iteration_time = thi[j].IterationTime[chosen_iteration-1];

      for (unsigned it = 0; it < max_iteration; it++)
      {
        thi[j].phases_factor.push_back (* (new vector<float>) );
        thi[j].phases_outsamples.push_back ( * (new vector<unsigned long long>) );
        thi[j].phases_total_counters.push_back( * (new vector<hwcmap>) );

        for (unsigned phase_num = 0; phase_num < thi[j].max_numphase; phase_num++)
        {
          float factor = (float)((thi[j].phases_length)[chosen_iteration-1])[phase_num]/(float)((thi[j].phases_length)[it])[phase_num];
          ((thi[j].phases_factor)[it]).push_back (factor);

          float length_factor = (float)((thi[j].phases_length)[chosen_iteration-1])[phase_num]/(float)total_iteration_time;
          float mul = length_factor * num_out_samples;

          thi[j].phases_outsamples[it].push_back ( lround(mul+1) );
          thi[j].phases_total_counters[it].push_back ( *(new hwcmap) );
        }
      }
    }

	p->parseBody();

#if 0
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

		}
	}
#endif

	//for (int i = 0; i < 1; i++)
	for (int i = 0; i < p->ih.getNumTasks(); i++)
	{
		TaskInformation *ti = p->ih.getTasksInformation();
		//for (int j = 0; j < 1; j++) 
		for (int j = 0; j < ti[i].getNumThreads(); j++) 
			p->dumpCounters_splitIteration (i, j);
	}

	outfile.close();

	return 0;
}
