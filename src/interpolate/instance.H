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
#ifndef INSTANCE_H_INCLUDED
#define INSTANCE_H_INCLUDED

#include "sample.H"
#include <vector>
#include <string>
#include <map>
#include <set>

using namespace std;

/* This class represents every instance extracted by the extract tool.
   It contains several information from the instance: name, which identifier
   application (ptask, task, thread), when it started, when it lasted, the 
   data objects living at the entry, PLUS:
      - samples
      - set of counters within the instance 
      - total counter per each of the previous counters */

class Instance
{
	private:
	/* Following are self-explanatory */
	const string RegionName;
	const unsigned ptask;
	const unsigned task;
	const unsigned thread;
	const unsigned long long startTime;
	const unsigned long long duration;
	const set<unsigned> livingDataObjects;
	unsigned long long prvValue;
	unsigned group;

	/* Container of samples */
	vector<Sample*> Samples;

	/* Which counters have been captured for this instance? */
	set<string> Counters;

	/* Which are the total counter values for these counters? */
	map<string, unsigned long long> TotalCounterValues;

	public:
	Instance (unsigned ptask, unsigned task, unsigned thread, string region,
	  unsigned long long startTime, unsigned long long duration,
	  set<string> &counters, map<string, unsigned long long> totalcountervalues,
	  const set<unsigned> & livingDataObjects);
	~Instance(void);

	/* Shows to terminal, debugging purposes */
	void Show(void);
	
    /* Checks whether a given counter was captured within the instance */
	bool hasCounter (const string & c) const;

	/* Adds a sample into the vector of samples */
	void addSample (Sample *s)
	  { Samples.push_back (s); }

	/* Get the vector of samples */
	vector<Sample*> getSamples(void)
	  { return Samples; }

	/* Get the # of samples */
	unsigned getNumSamples(void) const
	  { return Samples.size(); }

	/* Set a group identifier */
	void setGroup (unsigned g)
	  { this->group = g; }

	/* Get the group identifier  */
	unsigned getGroup (void) const
	  { return group; }

	/* Get the PRV associated value */
	unsigned long long getPRVvalue (void) const
	  { return prvValue; }

	/* Set the PRV associated value */
	void setPRVvalue (unsigned long long v)
	  { prvValue = v; }

	/* Get the duration for this instance */
	unsigned long long getDuration (void) const
	  { return duration; }

	/* Get the starting time for this instance */
	unsigned long long getStartTime (void) const
	  { return startTime; }

	/* Get the ptask from the paraver appl id */
	unsigned getptask (void) const
	  { return ptask; }

	/* Get the task from the paraver appl id */
	unsigned gettask (void) const
	  { return task; }

	/* Get the thread from the paraver appl id */
	unsigned getthread (void) const
	  { return thread; }

	/* For a given counter ctr, get its total value (at end of instance) */
	unsigned long long getTotalCounterValue (const string & ctr);

	/* Get the total counters (i.e. at the end) for every counters in the instance */
	map<string, unsigned long long> & getTotalCounterValues (void)
	  { return TotalCounterValues; }

	/* Get the name of the instance */
	string getRegionName(void) const
	  { return RegionName; }

	/* Get the living data objects at the begin of the instance */
	const set<unsigned> getLivingDataObjects (void) const
	  { return livingDataObjects; }
};

#endif


