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

#ifndef INSTANCE_H_INCLUDED
#define INSTANCE_H_INCLUDED

#include "sample.H"
#include <vector>
#include <string>
#include <map>
#include <set>

using namespace std;

class Instance
{
	private:
	const string RegionName;
	const unsigned ptask;
	const unsigned task;
	const unsigned thread;
	const unsigned long long startTime;
	const unsigned long long duration;
	unsigned long long prvValue;
	unsigned group;
	vector<Sample*> Samples;
	set<string> Counters;
	map<string, unsigned long long> TotalCounterValues;

	public:
	Instance (unsigned ptask, unsigned task, unsigned thread, string region,
	  unsigned long long startTime, unsigned long long duration,
	  set<string> &counters, map<string, unsigned long long> totalcountervalues);
	~Instance(void);

	void Show(void);
	
	bool hasCounter (const string & c) const;
	void addSample (Sample *s)
	  { Samples.push_back (s); }
	vector<Sample*> getSamples(void)
	  { return Samples; }
	unsigned getNumSamples(void) const
	  { return Samples.size(); }
	void setGroup (unsigned g)
	  { this->group = g; }
	unsigned getGroup (void) const
	  { return group; }
	unsigned long long getPRVvalue (void) const
	  { return prvValue; }
	void setPRVvalue (unsigned long long v)
	  { this->prvValue = v; }
	unsigned long long getDuration (void) const
	  { return duration; }
	unsigned long long getStartTime (void) const
	  { return startTime; }
	unsigned getptask (void) const
	  { return ptask; }
	unsigned gettask (void) const
	  { return task; }
	unsigned getthread (void) const
	  { return thread; }
	unsigned long long getTotalCounterValue (const string & ctr);
	map<string, unsigned long long> & getTotalCounterValues (void)
	  { return TotalCounterValues; }
	string getRegionName(void) const
	  { return RegionName; }
};

#endif


