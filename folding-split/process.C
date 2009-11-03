
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <vector>

using namespace std;

#include "process.H"

#include "globals.H"

using namespace domain;

Process::Process (string prvFile, bool multievents) : ParaverTrace (prvFile, multievents)
{
}

void Process::DumpParaverLine (fstream &f, unsigned long long type, unsigned long long value, unsigned long long time, unsigned long long task, unsigned long long thread)
{
	/* 2:14:1:14:1:69916704358:40000003:0 */
	f << "2:" << task << ":1:" << task << ":" << thread << ":" << time << ":" << type << ":" << value << endl;
}


void Process::DumpInformation (string traceprefix)
{
	ofstream dout ((traceprefix+".foldingmetadata").c_str(), ios::out);
	ofstream bout ((traceprefix+".boundaries").c_str(), ios::out);

	/* DUMP ITERATION INFO */
	dout << "ITERATION_EVENT " << iteration_event << endl;
	dout << "PHASE_EVENT " << phase_separator << endl;
	dout << "CHOSEN_ITERATION " << chosen_iteration << endl;
	dout << "NUM_TASKS " << ih.getNumTasks() << endl;

	/* Calculate num phases */
	int maxphases = 0;
	for (unsigned i = 0; i < ih.getNumTasks(); i++)
	{
		TaskInformation *ti = ih.getTasksInformation();
		for (unsigned j = 0; j < ti[i].getNumThreads(); j++)
		{
			ThreadInformation *thi = ti[i].getThreadsInformation();
			maxphases = MAX(maxphases, thi[j].max_numphase);
		}
	}

	/* Allocate structures for phases */
	vector<unsigned> vID;
	vector<unsigned long long> vIDtime_min;
	vector<unsigned long long> vIDtime_max;
	for (unsigned i = 0; i < maxphases; i++)
	{
		unsigned long long max_value = 1 << 31;
		max_value = max_value << 31;

		vID.push_back (0);
		vIDtime_min.push_back (max_value);
		vIDtime_max.push_back (0);
	}

	/* Compute boundaries across all tasks */
	for (unsigned i = 0; i < ih.getNumTasks(); i++)
	{
		TaskInformation *ti = ih.getTasksInformation();
		for (unsigned j = 0; j < ti[i].getNumThreads(); j++)
		{
			unsigned long long startcurrentphase = 0, startnextphase = 0;

			ThreadInformation *thi = ti[i].getThreadsInformation();
			for (unsigned phase = 0; phase < thi[j].max_numphase-1; phase++)
			{
				startcurrentphase += thi[j].phases_length[chosen_iteration-1][phase];
				startnextphase = startcurrentphase + thi[j].phases_length[chosen_iteration-1][phase+1];

				bool find_before = false;
				for (unsigned prevphase = 0; prevphase < phase; prevphase++)
					find_before = (thi[j].phases_map[prevphase] == thi[j].phases_map[phase]) || find_before;

				if (!find_before)
				{
					unsigned long long prevmax, prevmin;

					vID[phase] = thi[j].phases_map[phase];
					prevmax = vIDtime_max[phase];
					prevmin = vIDtime_min[phase];
					vIDtime_max[phase] = MAX(prevmax, startnextphase + thi[j].timeChosenIteration);
					vIDtime_min[phase] = MIN(prevmin, startcurrentphase + thi[j].timeChosenIteration);
				}
				
			}
		}
	}

	for (unsigned i = 0; i < vID.size(); i++)
	{
		if (vID[i] != 0)
		{
      unsigned long long delta, min, max;
      delta = ((vIDtime_max[i] - vIDtime_min[i])/100) * 5;
      if (vIDtime_min[i] < delta)
        min = 0;
      else
        min = vIDtime_min[i] - delta;
      max = vIDtime_max[i]+delta;

			bout << "FUNCTION " << vID[i] << " MINTIME " << min << " MAXTIME " << max << endl;
		}
	}

	for (unsigned i = 0; i < ih.getNumTasks(); i++)
	{
		TaskInformation *ti = ih.getTasksInformation();
		dout << "TASK " << i << " HAS " <<  ti[i].getNumThreads() << " THREADS" << endl;
		for (unsigned j = 0; j < ti[i].getNumThreads(); j++) 
		{
			ThreadInformation *thi = ti[i].getThreadsInformation();

			min_iteration = min_iteration;
			max_iteration = MIN(thi[j].IterationTime.size(), max_iteration);
			dout << "MIN_ITERATION " <<  min_iteration << " MAX_ITERATION " << max_iteration << endl;

			dout << "DURATION ";
			for (unsigned k = 1; k < min_iteration; k++)
				dout << "0 ";
			for (unsigned k = min_iteration; k <= max_iteration; k++)
				dout << thi[j].IterationTime[k-1] << " ";
			dout << endl;

			/* Dump excluded */
			dout << "EXCLUDED ";
			for (unsigned k = 1; k < min_iteration; k++)
				dout << "0 ";
			for (unsigned k = min_iteration; k <= max_iteration; k++)
				dout << thi[j].excluded_iteration[k-1] << " ";
			dout << endl;

			/* Dump times */
			dout << "NORMALIZATION_TIMES ";
			for (unsigned k = 1; k < min_iteration; k++)
				dout << "0.0 ";
			for (unsigned k = min_iteration; k <= max_iteration; k++)
				dout << thi[j].normalization_factor[k-1] << " ";
			dout << endl;

			/* Dump number of phases */
			dout << "MAX_NUMBER_OF_PHASES " << thi[j].max_numphase << endl;

			/* Dump max id in phase */
			dout << "MAX_VALUE_IN_PHASE " << thi[j].max_value_in_phase << endl;

			/* Dump phases map */
			dout << "PHASE_MAP ";
			for (unsigned phase = 0; phase < thi[j].max_numphase; phase++)
				dout << thi[j].phases_map[phase] << " ";
			dout << endl;

			/* Dump length of phases */
			dout << "PHASES_LENGTH ";
			for (unsigned it = 1; it <= max_iteration; it++)
				for (unsigned k = 0; k < thi[j].max_numphase; k++)
					dout << ((thi[j].phases_length)[it-1])[k] << " ";
			dout << endl;

			/* Dump phases */
			dout << "PHASES ";
			for (unsigned it = 1; it <= max_iteration; it++)
				for (unsigned k = 0; k < thi[j].max_numphase; k++)
					dout << ((thi[j].phases)[it-1])[k] << " ";
			dout << endl;
		}
	}
}

void Process::ReadInformation (string traceprefix)
{
	ifstream din ((traceprefix+".foldingmetadata").c_str(), ios::in);
	string s_tmp;
	long long int i_tmp;
	unsigned long long u_tmp;
	float f_tmp;


	/* READ ITERATION INFO */
	din >> s_tmp;
	din >> iteration_event;
#if defined(REDUMP)
	cout << "ITERATION_EVENT " << iteration_event << endl;
#endif /* REDUMP */

	din >> s_tmp;
	din >> phase_separator;
#if defined(REDUMP)
	cout << "PHASE_EVENT " << phase_separator << endl;
#endif /* REDUMP */

	din >> s_tmp;
	din >> chosen_iteration;
#if defined(REDUMP)
	cout << "CHOSEN_ITERATION " << chosen_iteration << endl;
#endif /* REDUMP */

	din >> s_tmp;
	din >> u_tmp;
#if defined(REDUMP)
	cout << "NUM_TASKS " << u_tmp << endl;
#endif /* REDUMP */
	ih.AllocateTasks (u_tmp);

	for (unsigned i = 0; i < ih.getNumTasks(); i++)
	{
		TaskInformation *ti = ih.getTasksInformation();

		din >> s_tmp;
		din >> i_tmp;
		din >> s_tmp;
		din >> u_tmp;
		din >> s_tmp;

#if defined(REDUMP)
		cout << "TASK " << i_tmp << " HAS " << u_tmp << " THREADS" << endl;
#endif /* REDUMP */
		ti[i].AllocateThreads (u_tmp);

		for (unsigned j = 0; j < ti[i].getNumThreads(); j++) 
		{
			ThreadInformation *thi = ti[i].getThreadsInformation();

			din >> s_tmp;
			din >> min_iteration;
			din >> s_tmp;
			din >> max_iteration;

#if defined(REDUMP)
			cout << "MIN_ITERATION " <<  min_iteration << " MAX_ITERATION " << max_iteration << endl;
#endif /* REDUMP */

			/* Read durations */
			thi[j].IterationTime.reserve (max_iteration);
			din >> s_tmp;
#if defined(REDUMP)
			cout << "DURATION ";
#endif /* REDUMP */
			for (unsigned k = 1; k <= max_iteration; k++)
			{
				din >> i_tmp;
				thi[j].IterationTime.push_back (i_tmp);
#if defined(REDUMP)
				cout << thi[j].IterationTime[k-1] << " ";
#endif /* REDUMP */
			}
#if defined(REDUMP)
			cout << endl;
#endif /* REDUMP */

			/* Read excluded */
			thi[j].excluded_iteration.reserve (max_iteration);
			din >> s_tmp;
#if defined(REDUMP)
			cout << "EXCLUDED ";
#endif /* REDUMP */
			for (unsigned k = 1; k <= max_iteration; k++)
			{
				din >> i_tmp;
				thi[j].excluded_iteration.push_back (i_tmp);
#if defined(REDUMP)
				cout << thi[j].excluded_iteration[k-1] << " ";
#endif /* REDUMP */
			}
#if defined(REDUMP)
			cout << endl;
#endif /* REDUMP */

			/* Read times */
			thi[j].normalization_factor.reserve (max_iteration);
			din >> s_tmp;
#if defined(REDUMP)
			cout << "NORMALIZATION_TIMES ";
#endif /* REDUMP */
			for (unsigned k = 1; k <= max_iteration; k++)
			{
				din >> f_tmp;
				thi[j].normalization_factor.push_back(f_tmp);
#if defined(REDUMP)
				cout << thi[j].normalization_factor[k-1] << " ";
#endif /* REDUMP */
			}
#if defined(REDUMP)
			cout << endl;
#endif /* REDUMP */

			/* Read number of phases */
			din >> s_tmp;
			din >> i_tmp;
			thi[j].max_numphase = i_tmp;
#if defined(REDUMP)
			cout << "MAX_NUMBER_OF_PHASES " << thi[j].max_numphase << endl;
#endif /* REDUMP */

			/* Read max id in phase */
			din >> s_tmp;
			din >> i_tmp;
			thi[j].max_value_in_phase = i_tmp;
#if defined(REDUMP)
			cout << "MAX_VALUE_IN_PHASE " << thi[j].max_value_in_phase << endl;
#endif /* REDUMP */

			/* Dump phases map */
			din >> s_tmp;
#if defined(REDUMP)
			cout << "PHASE_MAP ";
#endif
			thi[j].phases_map.reserve (thi[j].max_numphase);
			for (unsigned phase = 0; phase < thi[j].max_numphase; phase++)
			{
				din >> u_tmp;
				thi[j].phases_map.push_back (u_tmp);
#if defined(REDUMP)
				cout << thi[j].phases_map[phase] << " ";
#endif
			}
#if defined(REDUMP)
			cout << endl;
#endif

			/* Read phases length */
			thi[j].phases_length.reserve(max_iteration);
			din >> s_tmp;
#if defined(REDUMP)
			cout << "PHASES_LENGTH ";
#endif /* REDUMP */
			for (unsigned it = 1; it <= max_iteration; it++)
			{
				thi[j].phases_length.push_back ( * (new vector<unsigned long long>) );
				thi[j].phases_length[it-1].reserve (thi[j].max_numphase);
				for (unsigned k = 0; k < thi[j].max_numphase; k++)
				{
					din >> u_tmp;
					thi[j].phases_length[it-1].push_back (u_tmp);
#if defined(REDUMP)
					cout << (thi[j].phases_length[it-1])[k] << " ";
#endif /* REDUMP */
				}
			}
#if defined(REDUMP)
			cout << endl;
#endif /* REDUMP */

			/* Read phases */
			thi[j].phases.reserve (max_iteration);
			din >> s_tmp;
#if defined(REDUMP)
			cout << "PHASES ";
#endif /* REDUMP */
			for (unsigned it = 1; it <= max_iteration; it++)
			{
				thi[j].phases.push_back ( * (new vector<unsigned long long>) );
				thi[j].phases[it-1].reserve (thi[j].max_numphase);
				for (unsigned k = 0; k < thi[j].max_numphase; k++)
				{
					din >> u_tmp;

					/* HSG Dirty hack! :S */
					if (k == 0 && it > 1)
						thi[j].phases[it-2].push_back (u_tmp);

					thi[j].phases[it-1].push_back (u_tmp);
#if defined(REDUMP)
					cout << (thi[j].phases[it-1])[k] << " ";
#endif
				}
			}

#if defined(REDUMP)
			cout << endl;
#endif
		}
	}
}
