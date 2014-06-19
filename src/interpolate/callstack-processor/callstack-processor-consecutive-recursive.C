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

//#define DEBUG

CallstackProcessor_ConsecutiveRecursive_ProcessedInfo::CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
	unsigned r, double t, bool s)
	: routineId(r), time(t), speculated(s)
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

vector < pair < unsigned, double > >
CallstackProcessor_ConsecutiveRecursive::processSamples (
	const vector<Sample*> &samples,
	unsigned & first_depth)
{
#if defined(DEBUG)
	cout << "processSamples (or = " << openRecursion << ")" << endl;
#endif

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
				map<unsigned, CodeRefTriplet>::const_iterator i = callers.find(depth);
				if ((*i).second.getCaller() != 0)
				{
					found = true;
					break;
				}
			}
		}
		if (!found)
			depth--;
	}

	first_depth = depth;

	vector<CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > tmp = 
	  processSamples_r (samples, depth, 0, 0.0f, 1.0f, true); 

	vector< pair < unsigned, double> > r;
	for (auto & t : tmp)
	{
		r.push_back (make_pair (t->getRoutineId(), t->getTime()));
		delete t;
	}
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
	cout << "processSamples_r (" << stackdepth << "," << depth << "," << start << "," << end << ")" << endl;
#endif

	vector< CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > regions = 
	  searchConsecutiveRegions (samples, stackdepth, start, end, allow_speculated);
	assert (regions.size() % 2 == 0);

	res = regions;

#if defined(DEBUG)
	cout << "REGIONS found: " << regions.size() << " ";
	for (auto const & r : regions)
		cout << "<" << r->getRoutineId() << "," << r->getTime() << "," << r->getSpeculated() << "> ";
	cout << endl;
#endif

	if (stackdepth > 1 && regions.size() > 0)
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
	vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > res;

#if defined(DEBUG)
	cout << "searchConsecutiveRegions ( depth = " << stackdepth << 
	  " start = " << start << " end = " << end << ")" << endl;
#endif

	string time = common::DefaultTimeUnit;
	//string time = "PAPI_TOT_INS";

	/* attention, samples should be sorted by time! select first those that 
	   are within the region delimited by start - end */
	vector<Sample*> workSamples;
	for (auto s : samples)
//		if (s->getNTime() >= start && s->getNTime() <= end)
		if (ig->getInterpolatedNTime (time, s) >= start && 
		    ig->getInterpolatedNTime (time, s) <= end)
			workSamples.push_back (s);

	/* Search for first non-zero, and then guess that it will start from there */
	set<unsigned> foundcallers;
	for (auto s : workSamples)
	{
		const map<unsigned, CodeRefTriplet> & callers =
		  s->getCodeTripletsAsConstReference();
		if (callers.count (stackdepth) > 0)
			if (callers.at(stackdepth).getCaller() != 0)
				foundcallers.insert (callers.at(stackdepth).getCaller());
	}

#if defined(DEBUG)
	cout << "workSamples.size() = " << workSamples.size() << endl;
	for (auto s : workSamples)
	{
		const map<unsigned, CodeRefTriplet> & callers = s->getCodeTripletsAsConstReference();
		if (callers.count (stackdepth) > 0)
		{
			map<unsigned, CodeRefTriplet>::const_iterator i = callers.find(stackdepth);
			if ((*i).second.getCaller() != 0)
//				cout << s->getNTime() << " " << (*i).second.getCaller() << endl;
				cout << ig->getInterpolatedNTime (time, s) << " " << (*i).second.getCaller() << endl;
		}
	}
	cout << "foundcallers.size() = " << foundcallers.size() << " workSamples.size() = " << workSamples.size() << endl;
#endif

	if (foundcallers.size() == 1)
	{
		/* Only one caller found, consider it covers the whole region
		   start-end only if speculateds are allowed */
		if (allow_speculated)
		{
			unsigned caller = *(foundcallers.cbegin());
			res.push_back ( new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (caller, start, true) );
			res.push_back ( new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (0, end, true) );
		}
	}
	else if (foundcallers.size() > 1)
	{
		CallastackProcessor_ConsecutiveRecursive_ConsecutiveCtrl *ctrl =
		  new CallastackProcessor_ConsecutiveRecursive_ConsecutiveCtrl (nConsecutiveSamples);

		bool in_region = false;
		double entry_time_in_region;
		unsigned caller_in_region;

		for (auto s : workSamples)
		{
			const map<unsigned, CodeRefTriplet> & callers =
			  s->getCodeTripletsAsConstReference();
			if (callers.count (stackdepth) > 0)
			{
				map<unsigned, CodeRefTriplet>::const_iterator i = 
				  callers.find(stackdepth);
				if ((*i).second.getCaller() != 0)
				{
					// ctrl->add (make_pair ((*i).second.getCaller(), s->getNTime()));
					ctrl->add (make_pair ((*i).second.getCaller(), ig->getInterpolatedNTime (time, s)));

					if (!in_region)
					{
						if (ctrl->allEqual())
						{
#if defined(DEBUG)
							cout << "Entering at " << (*i).second.getCaller() << " @ " << ctrl->getFirstTime() << endl;
#endif
							in_region = true;
							entry_time_in_region = ctrl->getFirstTime();
							caller_in_region = (*i).second.getCaller();
						}
					}
					else
					{
						if (!ctrl->allEqual())
						{
#if defined(DEBUG)
//							cout << "Leaving @ " << s->getNTime() << endl;
							cout << "Leaving @ " << ig->getInterpolatedNTime (time, s) << endl;
							ctrl->show();
#endif
							in_region = false;
							// if (s->getNTime() - entry_time_in_region > openRecursion)
							if (ig->getInterpolatedNTime (time, s) - entry_time_in_region > openRecursion)
							{
								// res.push_back (make_pair (caller_in_region, entry_time_in_region));
								// // res.push_back (make_pair (0, s->getNTime()));
								//res.push_back (make_pair (0, ig->getInterpolatedNTime (time, s)));

								res.push_back ( new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (caller_in_region, entry_time_in_region, false)  );
								res.push_back ( new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (0, ig->getInterpolatedNTime (time, s), false) );
							}
						}
					}
				}
			}
		}

		/* if we leave within a region, close it */
#if defined(DEBUG)
		cout << "Leaving @ " << end << endl;
		ctrl->show();
#endif
		if (in_region && end - entry_time_in_region > openRecursion)
		{
			res.push_back ( new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (caller_in_region, entry_time_in_region, false)  );
			res.push_back ( new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (0, end, false) );
		}

		delete ctrl;
	}

	return res;
}

