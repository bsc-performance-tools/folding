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

#include <assert.h>
#include "callstack-processor-consecutive-recursive-ctrl.H"

#include <iostream>

CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl::CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl (
	unsigned nConsecutiveSamples) : nConsecutiveSamples(nConsecutiveSamples)
{
}

void CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl::add (
	const pair<CodeRefTriplet,double> & callertime)
{
	/* Keep at most up to nConsecutiveSamples in the deque plus their timings */
	callers_times.push_back (callertime);
	if (callers_times.size () > nConsecutiveSamples)
		callers_times.pop_front();

	assert (callers_times.size() <= nConsecutiveSamples);
}

bool CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl::allEqual (void) const
{
	unsigned headCaller = (*(callers_times.cbegin())).first.getCaller();
	for (auto caller : callers_times)
		if (caller.first.getCaller() != headCaller)
			return false;
	return true;
}

void CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl::show (void) const
{
	cout << "CTRL contents: ";
	for (const auto caller : callers_times)
		cout << " " << caller.first.getCaller() << "@" << caller.second;
	cout << endl;
}

double CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl::getFirstTime (void) const
{
	assert (callers_times.size() > 0);
	return (*(callers_times.cbegin())).second;
}

double CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl::getLastTime (void) const
{
	if (callers_times.empty())
		return 0.;
	else
		return (*(callers_times.crbegin())).second;
}

CodeRefTriplet CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl::getFirstCodeRef (void) const
{
	assert (callers_times.size() > 0);
	return (*(callers_times.cbegin())).first;
}
