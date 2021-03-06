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
#include "instance.H"
#include <iostream>
#include <algorithm>
#include <assert.h>


/* Class constructor */
Instance::Instance (unsigned ptask, unsigned task, unsigned thread,
	string region, unsigned long long startTime, unsigned long long duration,
	set<string> &counters, map<string, unsigned long long> totalcountervalues,
	const set<unsigned> &dataobjects)
	: RegionName(region), ptask(ptask), task(task), thread(thread),
	startTime(startTime), duration(duration), livingDataObjects(dataobjects)
{
	this->group = 0;
	this->prvValue = 0;
	this->Counters = counters;
	this->TotalCounterValues = totalcountervalues;
}

/* Class destructor */
Instance::~Instance (void)
{
	for (unsigned s = 0; s < Samples.size(); s++)
		delete (Samples[s]);
	Samples.clear();
}

/* Show instance, for debugging purposes */
void Instance::Show (void)
{
	cout << "instance info: " << endl;
	cout << "rname = " << RegionName << endl; 
	cout << "ptask = " << ptask << " task = " << task << " thread = " << thread << endl;
	cout << "group = " << group << " starttime = " << startTime << " duration = " << duration << " prvValue = " << prvValue << endl;
	set<string>::iterator it;
	cout << "COUNTERS: ";
	for (it = Counters.begin(); it != Counters.end(); it++)
		cout << *it << " ";
	cout << endl;
}

bool Instance::hasCounter (const string &ctr) const
{
	return TotalCounterValues.count(ctr) > 0;
}

unsigned long long Instance::getTotalCounterValue (const string &ctr)
{
	assert (hasCounter(ctr));
	return TotalCounterValues[ctr];
}
