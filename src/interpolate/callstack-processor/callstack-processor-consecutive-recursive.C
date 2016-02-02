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
  CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (unsigned l,
  CodeRefTriplet coderef, double t, bool s)
	: level(l), CodeRef(coderef), time(t), speculated(s)
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

CallstackProcessor_ConsecutiveRecursive::~CallstackProcessor_ConsecutiveRecursive ()
{
}

vector < CallstackProcessor_Result* >
CallstackProcessor_ConsecutiveRecursive::processSamples (
	const vector<Sample*> &samples)
{
#if defined(DEBUG)
	cout << "processSamples (or = " << openRecursion << ", samples.size = " << samples.size() << ")" << endl;
#endif

	cout << std::setprecision(10) << endl;

#if defined(DEBUG)
	cout << "looking for max depth in samples" << endl;
#endif

	/* Look for the max depth of the samples */
	unsigned max_depth = 0;
	for (auto s : samples)
		max_depth = MAX(max_depth, s->getMaxCallerLevel());

#if defined(DEBUG)
	cout << "max depth = " << max_depth << endl;
	cout << "looking for first all-non-zeros" << endl;
#endif

	/* Look for the first level that has non-all zeros at the bottom level,
	   this should be at max_depth, but just in case */
	unsigned depth = max_depth;
	bool found = false;
	while (!found)
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

#if defined(DEBUG)
	cout << "found = " << found << " depth = " << depth << endl;
#endif

	vector<CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > tmp = 
	  processSamples_r (samples, depth, 0, 0.0f, 1.0f, true); 

	vector< CallstackProcessor_Result* > r;
	for (auto & t : tmp)
	{
		r.push_back (
		  new CallstackProcessor_Result (
		    t->getLevel(), t->getCodeRef(), t->getTime()
		  )
		);
		delete t;
	}

#if defined(DEBUG) 
	cout << "processSamples OUTPUT ("<< r.size()<< "):: " << endl;
	unsigned d = 0;
	for (const auto rinfo : r)
	{
		if (rinfo->getCodeRef().getCaller() > 0)
			d++;
		for (int i = 0; i < d; i++)
			cout << "  ";
	  	cout << "[" << rinfo->getLevel() << "," << rinfo->getCodeRef().getCaller() <<
		     "," << rinfo->getNTime() << "] " << endl;
		if (rinfo->getCodeRef().getCaller() == 0)
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
		cout << "<" << r->getCodeRef().getCaller() << "," << r->getTime() << "," 
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

CodeRefTriplet CallstackProcessor_ConsecutiveRecursive::SearchMostFrequent (
	const map<CodeRefTriplet, unsigned> & f)
{
	assert (f.size() > 0);

	unsigned maxfreq = 0;
	CodeRefTriplet res;
	for (auto const & elem : f)
		if (elem.first.getCaller() != 0 && elem.second > maxfreq)
		{
			res = elem.first;
			maxfreq = elem.second;
		}
	return res;
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

	vector < pair < CodeRefTriplet, double > > vCallerTime;

#if defined(TIME_BASED_COUNTER)
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
			{
				caller = callers.at(stackdepth).getCaller();
				vCallerTime.push_back (
				  make_pair (callers.at(stackdepth), sample_time));
				all_zeroes = all_zeroes && caller == 0;
			}
			else
			{
				CodeRefTriplet crt;
				vCallerTime.push_back (
					make_pair (crt, sample_time));
				all_zeroes = false;
			}
		}
	}

	/* Empty or everything is 0?, return empty */
	if (vCallerTime.size() == 0 || all_zeroes)
		return res;

	CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl *ctrl =
	  new CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl (nConsecutiveSamples);

	/* Use a hash to look for the most present CodeRefTriplet within a region */
	map<CodeRefTriplet,unsigned> CodeRefFrequency;

	bool in_region = false;
	double entry_time_in_region = 0;
	CodeRefTriplet crt_in_region;

	for (const auto & ct : vCallerTime)
	{
		double last_time = ctrl->getLastTime();

		ctrl->add (ct);

		// If we're inside a region, add this into coderef statistics
		if (in_region)
		{
			if (CodeRefFrequency.count(ct.first) != 0)
				CodeRefFrequency[ct.first]++;
			else
				CodeRefFrequency[ct.first] = 1;
		}

#if defined(DEBUG)
		cout << "Caller: " << ct.first.getCaller() << " @ " << ct.second << endl;
#endif

		if (!in_region && ctrl->allEqual())
		{
#if defined(DEBUG)
			cout << "Entering at " << ct.first.getCaller() << " @ " << ct.second << endl;
			ctrl->show();
#endif
			in_region = true;
			entry_time_in_region = ctrl->getFirstTime();
			crt_in_region = ctrl->getFirstCodeRef();

			// Restart statistics
			CodeRefFrequency.clear();

			// Add these consecutive found into the statistics
			CodeRefFrequency[ct.first] = nConsecutiveSamples;

		}
		else if (in_region && !ctrl->allEqual())
		{
#if defined(DEBUG)
			cout << "Leaving @ " << ct.second << endl;
			ctrl->show();
#endif
			in_region = false;
			if (crt_in_region.getCaller() != 0 && ct.second - entry_time_in_region > openRecursion)
			{
				CodeRefTriplet entry_crt = SearchMostFrequent (CodeRefFrequency);
				CodeRefTriplet exit_crt;

				res.push_back (
				  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
				    stackdepth, entry_crt, entry_time_in_region, false)  );
				res.push_back (
				  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
				    stackdepth, exit_crt, last_time, false) );
			}
		}
	}

#if defined(DEBUG)
	cout << "Leaving @ " << end << endl;
	ctrl->show();
#endif

	/* if we leave within a region, close it at the end */
	if (in_region && end - entry_time_in_region > openRecursion && crt_in_region.getCaller() != 0)
	{
		CodeRefTriplet entry_crt = SearchMostFrequent (CodeRefFrequency);
		CodeRefTriplet exit_crt;

		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, entry_crt, entry_time_in_region, false)  );
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, exit_crt, end, false) );
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

	vector < pair < CodeRefTriplet, double > > vCallerTime;

#if defined(TIME_BASED_COUNTER)
	//string time = common::DefaultTimeUnit;
	string time = "PAPI_TOT_INS";
#endif /* TIME_BASED_COUNTER */

	/* Use a hash to look for the most present CodeRefTriplet within a region */
	map<CodeRefTriplet,unsigned> CodeRefFrequency;

	/* attention, samples should be sorted by time! select first those that 
	   are within the region delimited by start - end */
	set<CodeRefTriplet> seen_crts;
	bool all_zeroes = true;
	bool seen_zero = false;
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
			{
				caller = callers.at(stackdepth).getCaller();

				all_zeroes = all_zeroes && caller == 0;
				seen_zero = seen_zero || caller == 0;

				vCallerTime.push_back (
				  make_pair (callers.at(stackdepth), sample_time));

				seen_crts.insert (callers.at(stackdepth));

				if (CodeRefFrequency.count(callers.at(stackdepth)) > 0)
					CodeRefFrequency[callers.at(stackdepth)]++;
				else
					CodeRefFrequency[callers.at(stackdepth)] = 1;
			}
			else
			{
				CodeRefTriplet crt;
				vCallerTime.push_back (
					make_pair (crt, sample_time));

				all_zeroes = false;
				seen_zero = true;

				seen_crts.insert (crt);
			}
#if defined(DEBUG)
			s->show(false);
#endif
		}
	}

#if defined(DEBUG)
	cout << "SEEN CALLERS: ";
	for (auto const c : seen_crts)
		cout << c.getCaller() << " ";
	cout << endl;
#endif

	/* Empty or everything is 0?, return empty */
	if (vCallerTime.size() == 0 || all_zeroes)
		return res;

	/* If only one value is seen, or two (including 0),
	   return the whole area */	
	if (seen_crts.size() == 1)
	{
		/* One only? Then do not speculate from here */

		CodeRefTriplet entry_crt = SearchMostFrequent (CodeRefFrequency);
		CodeRefTriplet exit_crt;

		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, entry_crt, start, false) );
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, exit_crt, end, false) );

		return res;
	}
	else if (seen_crts.size() == 2 && seen_zero)
	{
		/* Two? Allow speculation */

		CodeRefTriplet entry_crt = SearchMostFrequent (CodeRefFrequency);
		CodeRefTriplet exit_crt;

		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, entry_crt, start, true) );
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, exit_crt, end, true) );
		return res;
	}

	CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl *ctrl =
	  new CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl (nConsecutiveSamples);

	bool in_region = false;
	double entry_time_in_region = 0;
	CodeRefTriplet crt_in_region;

	CodeRefFrequency.clear();

	for (const auto & ct : vCallerTime)
	{
#if defined(DEBUG)
		cout << "Caller: " << ct.first.getCaller() << " @ " << ct.second << endl;
#endif

		double last_time = ctrl->getLastTime();

		if (ct.first.getCaller() == 0)
			continue;

		ctrl->add (ct);

		// If we're inside a region, add this into coderef statistics
		if (in_region)
		{
			if (CodeRefFrequency.count(ct.first) != 0)
				CodeRefFrequency[ct.first]++;
			else
				CodeRefFrequency[ct.first] = 1;
		}

		if (!in_region && ctrl->allEqual())
		{
#if defined(DEBUG)
			cout << "Entering at " << ct.first.getCaller() << " @ " << ct.second << endl;
			ctrl->show();
#endif
			in_region = true;
			entry_time_in_region = ctrl->getFirstTime();
			crt_in_region = ctrl->getFirstCodeRef();

			// Reset frequency stats
			CodeRefFrequency.clear();

			// Add these consecutive found into the statistics
			CodeRefFrequency[ct.first] = nConsecutiveSamples;
		}
		else if (in_region && !ctrl->allEqual() && crt_in_region.getCaller() != 0)
		{
#if defined(DEBUG)
			cout << "Leaving @ " << ct.second << endl;
			ctrl->show();
#endif
			in_region = false;
			if (crt_in_region.getCaller() != 0 && ct.second - entry_time_in_region > openRecursion)
			{
				CodeRefTriplet entry_crt = SearchMostFrequent (CodeRefFrequency);
				CodeRefTriplet exit_crt;

				res.push_back (
				  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
				    stackdepth, entry_crt, entry_time_in_region, false)  );
				res.push_back (
				  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
				    stackdepth, exit_crt, last_time, false) );
			}
		}
	}

#if defined(DEBUG)
	cout << "Leaving @ " << end << endl;
	ctrl->show();
#endif

	/* if we leave within a region, close it at the end */
	if (in_region && end - entry_time_in_region > openRecursion && crt_in_region.getCaller() != 0)
	{
		CodeRefTriplet entry_crt = SearchMostFrequent (CodeRefFrequency);
		CodeRefTriplet exit_crt;

		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, entry_crt, entry_time_in_region, false) );
		res.push_back (
		  new CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (
		    stackdepth, exit_crt, end, false) );
	}

	delete ctrl;

	return res;
}

