#ifndef THREADINFORMATION_H
#define THREADINFORMATION_H

#include "FunctionBursts.h"

#include <list>
#include <map>
#include <vector>

using namespace std;

namespace domain
{

class TimeValue
{
	public:
	unsigned long long realvalue;
	unsigned long long value;
	unsigned long long time;
//	unsigned long long phase_time;
	unsigned long long iteration;
	unsigned long long absolute;
	unsigned long long phase;
	unsigned sample_address;
	unsigned sample_call;
	float normalized_time;
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
	vector<unsigned long long> IterationTime;

	vector<unsigned long long> numphases;
	unsigned long long max_numphase;
	vector<vector<unsigned long long> > phases;
	vector<vector<unsigned long long> > phases_from_iteration;
	vector<vector<unsigned long long> > phases_length;
	vector<vector<unsigned long long> > phases_outsamples;
	vector<unsigned long long> phases_map;
	vector<vector<hwcmap> > phases_total_counters;
	vector<vector<float> > phases_factor;
	unsigned current_phase;
	unsigned long long current_phase_time;

	vector<hwcmap> total_counters; /* map of counters at the end of each iteration */
	vector<bool> excluded_iteration;
	vector<bool> greatIPC;

	FunctionBursts fb;

	unsigned long long max_value_in_phase;
	unsigned long long currentIteration;
	unsigned long long currentPhase;
	unsigned long long lastTime;
	unsigned long long lastIterationTime;
	unsigned long long timeChosenIteration;
	unsigned long long lastMPItime;
	bool FirstIterationOcurred;
	bool inMPI;

	ThreadInformation ()
	{ 
		FirstIterationOcurred = inMPI = false;
		lastTime = lastIterationTime = currentPhase = currentIteration = timeChosenIteration = 0;
		current_phase = current_phase_time = 0;
		max_value_in_phase = 0;
		max_numphase = 0;
		lastMPItime = 0;
	};
};

} /* end of namespace domain */

#endif
