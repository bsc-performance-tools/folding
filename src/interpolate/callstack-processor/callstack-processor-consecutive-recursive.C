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
#include <algorithm>
#include "callstack-processor-consecutive-recursive.H"
#include "callstack-processor-consecutive-recursive-ctrl.H"
#include "instance-group.H"

#include <iostream>
#include <iomanip>

// #define DEBUG

CallstackProcessor_ConsecutiveRecursive_ProcessedInfo::
  CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (unsigned l, unsigned r,
  double t, bool s)
	: level(l), caller(r), time(t), speculated(s)
{
}


CallstackProcessor_ConsecutiveRecursive::CallstackProcessor_ConsecutiveRecursive (
	InstanceGroup *ig, unsigned nConsecutiveSamples, double openRecursion) 
	: CallstackProcessor(ig), nConsecutiveSamples(nConsecutiveSamples), 
	  openRecursion(openRecursion)
{
	assert (ig != NULL);
}

CallstackProcessor_ConsecutiveRecursive::CallstackProcessor_ConsecutiveRecursive (
	InstanceGroup *ig, unsigned nConsecutiveSamples, unsigned long long duration) 
	: CallstackProcessor(ig), nConsecutiveSamples(nConsecutiveSamples), 
	  openRecursion(((double) duration)/ig->mean())
{
	assert (ig != NULL);
}

vector < CallstackProcessor_Result* >
CallstackProcessor_ConsecutiveRecursive::processSamples (
	const vector<Sample*> &samples)
{
#if defined(DEBUG)
	cout << "processSamples (or = " << openRecursion << ")" << endl;
#endif

	cout << std::setprecision(10) << endl;

	/* Look for the max depth of the samples */
	unsigned max_depth = 0;
	for (auto s : samples)
		max_depth = MAX(max_depth, s->getMaxCallerLevel());

	/* Look for the first level that has non-all zeros at the bottom level,
	   this should be at max_depth, but just in case */
	unsigned depth = max_depth;
	bool found = false;
	while (depth >= 0 && !found)
	{
		for (auto s : samples)
		{
			const map<unsigned, CodeRefTriplet> & callers =
			  s->getCodeTripletsAsConstReference();
			if (callers.count (depth) > 0)
			{
				const CodeRefTriplet crt = callers.at(depth);
				if (crt.getCaller() != 0)
				{
					found = true;
					break;
				}
			}
		}
		if (!found)
			depth--;
	}

	vector<CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > tmp = 
	  processSamples_r (samples, depth, 0, 0.0f, 1.0f, true); 

	vector< CallstackProcessor_Result* > r;
	for (auto & t : tmp)
	{
		r.push_back (
		  new CallstackProcessor_Result (
		    t->getLevel(), t->getCaller(), t->getTime()
		  )
		);
		delete t;
	}

#if defined(DEBUG) 
	cout << "processSamples OUTPUT ("<< r.size()<< "):: " << endl;
	unsigned d = 0;
	for (const auto rinfo : r)
	{
		if (rinfo->getCaller() > 0)
			d++;
		for (int i = 0; i < d; i++)
			cout << "  ";
	  	cout << "[" << rinfo->getLevel() << "," << rinfo->getCaller() <<
		     "," << rinfo->getNTime() << "] " << endl;
		if (rinfo->getCaller() == 0)
			d--;
	}
	cout << endl;
#endif

	return r;
}

static bool compare_RegionsByTime (
	const CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * r1,
	const CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * r2)
{
	return r1->getTime() < r2->getTime();
}

vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * >
 CallstackProcessor_ConsecutiveRecursive::processSamples_r (
	const vector<Sample*> &samples, unsigned stackdepth, unsigned depth, 
	double start, double end, bool allow_speculated)
{
	vector<CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > res;

#if defined(DEBUG)
	cout << "processSamples_r (" << stackdepth << "," << depth << ","
	     << start << "," << end << ", " << allow_speculated << ")" << endl;
#endif

	vector< CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > regions = 
	  searchConsecutiveRegions (samples, stackdepth, start, end, allow_speculated);
	assert (regions.size() % 2 == 0);

	res = regions;

#if defined(DEBUG)
	cout << "REGIONS found: " << regions.size() << " ";
	for (auto const & r : regions)
		cout << "<" << r->getCaller() << "," << r->getTime() << "," 
		     << r->getSpeculated() << "> ";
	cout << endl;
#endif

	if (stackdepth > 0 && regions.size() > 0)
	{
		vector< CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * >::const_iterator it = regions.cbegin();
		while (it != regions.cend())
		{
			double start_region = (*it)->getTime();
			it++;
			double end_region = (*it)->getTime();

#if defined(DEBUG)
			cout << "RANGE [ " << start_region <<  "," << end_region << "]" << endl;
#endif

			if (end_region - start_region > openRecursion)
			{
#if defined(DEBUG)
				cout << stackdepth << " LOOKING For subregions in " << start_region << "," << end_region << endl;
#endif

				vector< CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > tmp = 
				  processSamples_r (samples, stackdepth-1, depth+1,
				    start_region, end_region, (*it)->getSpeculated());
				res.insert (res.end(), tmp.begin(), tmp.end());
			}
			it++;
		}
	}

	sort (res.begin(), res.end(), ::compare_RegionsByTime	);

	return res;
}

vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * >
 CallstackProcessor_ConsecutiveRecursive::searchConsecutiveRegions (
	const vector<Sample*> &samples, unsigned stackdepth, double start,
	double end, bool allow_speculated)
{
	return (allow_speculated)?
		searchConsecutiveRegions_speculated (samples, stackdepth, start, end)
		:
		searchConsecutiveRegions_nonspeculated (samples, stackdepth, start, end);

}

vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * >
 CallstackProcessor_ConsecutiveRecursive::searchConsecutiveRegions_nonspeculated (
	const vector<Sample*> &samples, unsigned stackdepth, double start,
	double end)
{
	vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > res;

#if defined(DEBUG)
	cout << "searchConsecutiveRegions_nonspeculated (depth = " << stackdepth << 
	  " start = " << start << " end = " << end << ")" << endl;
#endif

	vector < pair < unsigned, double > > vCallerTime;

#if defined(TIME_BASE_COUNTER)
	//string time = common::DefaultTimeUnit;
	string time = "PAPI_TOT_INS";
#endif /* TIME_BASED_COUNTER */

	/* attention, samples should be sorted by time! select first those that 
	   are within the region delimited by start - end */
	bool all_zeroes = true;
	for (auto s : samples)
	{
#if defined(TIME_BASED_COUNTER)
		double sample_time = ig->getInterpolatedNTime (time, s);
#else
		double sample_time = s->getNTime();
#endif /* TIME_BASED_COUNTER */

		if (sample_time > start && sample_time <= end)
		{
			unsigned caller = 0;
			const map<unsigned, CodeRefTriplet> & callers = s->getCodeTripletsAsConstReference();
			if (callers.count (stackdepth) > 0)
				caller = callers.at(stackdepth).getCaller();

			all_zeroes = all_zeroes && caller == 0;

			vCallerTime.push_back (make_pair (caller, sample_time));
		}
	}

	/* Empty or everything is 0?, return empty */
	if (vCallerTime.size() == 0 || all_zeroes)
		return res;

	CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl *ctrl =
	  new CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl (nConsecutiveSamples);

	bool in_region = false;
	double entry_time_in_region;
	unsigned caller_in_region;

	for (const auto & ct : vCallerTime)
	{
		double last_time = ctrl->getLastTime();

		ctrl->add (ct);

#if defined(DEBUG)
		cout << "Caller: " << ct.first << " @ " << ct.second << endl;
#endif

		if (!in_region && ctrl->allEqual())
		{
#if defined(DEBUG)
			cout << "Entering at " << ct.first << " @ " << ct.second << endl;
			ctrl->show();
#endif
			in_region = true;
			entry_time_in_region = ctrl->getFirstTime();
			caller_in_region = ctrl->getFirstCaller();
		}
		else if (in_region && !ctrl->allEqual())
		{
#if defined(DEBUG)
			cout << "Leaving @ " << ct.second << endl;
			ctrl->show();
#endif
			in_region = false;
			if (caller_in_region != 0 && ct.second - entry_time_in_region > openRecursion)
			{
				res.push_back (
				  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
				    stackdepth, caller_in_region, entry_time_in_region, false)  );
				res.push_back (
				  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
				    stackdepth, 0, last_time, false) );
			}
		}
	}

#if defined(DEBUG)
	cout << "Leaving @ " << end << endl;
	ctrl->show();
#endif

	/* if we leave within a region, close it at the end */
	if (in_region && end - entry_time_in_region > openRecursion && caller_in_region != 0)
	{
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, caller_in_region, entry_time_in_region, false)  );
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, 0, end, false) );
	}

	delete ctrl;

	return res;
}

vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * >
 CallstackProcessor_ConsecutiveRecursive::searchConsecutiveRegions_speculated (
	const vector<Sample*> &samples, unsigned stackdepth, double start,
	double end)
{
	vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > res;

#if defined(DEBUG)
	cout << "searchConsecutiveRegions_speculated (depth = " << stackdepth << 
	  " start = " << start << " end = " << end << ")" << endl;
#endif

	vector < pair < unsigned, double > > vCallerTime;

#if defined(TIME_BASED_COUNTER)
	//string time = common::DefaultTimeUnit;
	string time = "PAPI_TOT_INS";
#endif /* TIME_BASED_COUNTER */

	/* attention, samples should be sorted by time! select first those that 
	   are within the region delimited by start - end */
	set<unsigned> seen_callers;
	bool all_zeroes = true;
	for (auto s : samples)
	{
#if defined(TIME_BASED_COUNTER)
		double sample_time = ig->getInterpolatedNTime (time, s);
#else
		double sample_time = s->getNTime();
#endif /* TIME_BASED_COUNTER */

		if (sample_time > start && sample_time <= end)
		{
			unsigned caller = 0;
			const map<unsigned, CodeRefTriplet> & callers = s->getCodeTripletsAsConstReference();
			if (callers.count (stackdepth) > 0)
				caller = callers.at(stackdepth).getCaller();

			all_zeroes = all_zeroes && caller == 0;

			vCallerTime.push_back (make_pair (caller, sample_time));
#if defined(DEBUG)
			s->show(false);
#endif
			seen_callers.insert (caller);
		}
	}

#if defined(DEBUG)
	cout << "SEEN CALLERS: ";
	for (auto const c : seen_callers)
		cout << c << " ";
	cout << endl;
#endif

	/* Empty or everything is 0?, return empty */
	if (vCallerTime.size() == 0 || all_zeroes)
		return res;

	/* If only one value is seen, or two (including 0),
	   return the whole area */	
	if (seen_callers.size() == 1)
	{
		/* One only? Then do not speculate from here */
		set<unsigned>::const_iterator i = seen_callers.cbegin();
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, *i, start, false) );
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, 0, end, false) );
		return res;
	}
	else if (seen_callers.size() == 2 && seen_callers.count (0) == 1)
	{
		/* Two? Allow speculation */
		set<unsigned>::const_iterator i = seen_callers.cbegin();
		i++; /* First is always 0, skip it */
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, *i, start, true) );
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, 0, end, true) );
		return res;
	}

	CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl *ctrl =
	  new CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl (nConsecutiveSamples);

	bool in_region = false;
	double entry_time_in_region;
	unsigned caller_in_region;

	for (const auto & ct : vCallerTime)
	{
#if defined(DEBUG)
		cout << "Caller: " << ct.first << " @ " << ct.second << endl;
#endif

		double last_time = ctrl->getLastTime();

		if (ct.first == 0)
			continue;

		ctrl->add (ct);

		if (!in_region && ctrl->allEqual())
		{
#if defined(DEBUG)
			cout << "Entering at " << ct.first << " @ " << ct.second << endl;
			ctrl->show();
#endif
			in_region = true;
			entry_time_in_region = ctrl->getFirstTime();
			caller_in_region = ctrl->getFirstCaller();
		}
		else if (in_region && !ctrl->allEqual() && caller_in_region != 0)
		{
#if defined(DEBUG)
			cout << "Leaving @ " << ct.second << endl;
			ctrl->show();
#endif
			in_region = false;
			if (caller_in_region != 0 && ct.second - entry_time_in_region > openRecursion)
			{
				res.push_back (
				  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
				    stackdepth, caller_in_region, entry_time_in_region, false)  );
				res.push_back (
				  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
				    stackdepth, 0, last_time, false) );
			}
		}
	}

#if defined(DEBUG)
	cout << "Leaving @ " << end << endl;
	ctrl->show();
#endif

	/* if we leave within a region, close it at the end */
	if (in_region && end - entry_time_in_region > openRecursion && caller_in_region != 0)
	{
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, caller_in_region, entry_time_in_region, false)  );
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, 0, end, false) );
	}

	delete ctrl;

	return res;
}

