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

#include <iostream>
#include <time.h>

#include "interpolation.H"

Interpolation::Interpolation (unsigned steps, bool prefilter) : steps(steps),
	prefilter(prefilter)
{
}

void Interpolation::pre_interpolate (double /* sigmaTimes*/,
	InstanceGroup* /* ig */, const set<string> & /* counters */)
{
	/* By default, do nothing */
	return;
}

InterpolationResults * Interpolation::interpolate_kernel (vector<Sample*> vs,
	const string & counter, const string & region, unsigned group)
{
	InterpolationResults *res = new InterpolationResults(steps);
	res->setInterpolationDetails (details());

	unsigned incount = 0;
	for (unsigned s = 0; s < vs.size(); s++)
		if (vs[s]->hasCounter(counter))
			incount++;

	double * inpoints_x = new double [incount+2];
	double * inpoints_y = new double [incount+2];

	bool all_zeroes = true;
	inpoints_x[0] = inpoints_y[0] = 0;
	for (unsigned i = 1, s = 0; s < vs.size(); s++)
		if (vs[s]->hasCounter (counter))
		{
			inpoints_x[i] = vs[s]->getNTime();
			inpoints_y[i] = vs[s]->getNCounterValue (counter);
			all_zeroes = all_zeroes && inpoints_y[i] == 0;
			i++;
		}
	inpoints_x[incount+1] = inpoints_y[incount+1] = 1;

	cout << "   - Counter " << counter << ", # in samples = " << incount << flush;

	struct timespec time_start, time_end;
	clock_gettime (CLOCK_REALTIME, &time_start);
	{
		if (!all_zeroes)
		{
			do_interpolate (incount+2, inpoints_x, inpoints_y, res, counter,
			  region, group);

			/* Remove values below 0 */
			double * outpoints = res->getInterpolationResultsPtr();
			for (unsigned u = 0; u < steps; u++)
				if (outpoints[u] < 0)
					outpoints[u] = 0;
		}
		else
		{
			double * outpoints = res->getInterpolationResultsPtr();
			for (unsigned u = 0; u < steps; u++)
				outpoints[u] = 0;
		}
	}
	clock_gettime (CLOCK_REALTIME, &time_end);
	time_t delta = time_end.tv_sec - time_start.tv_sec;
	cout << ", elapsed time " << delta / 60 << "m " << delta % 60 << "s" << endl;

	delete [] inpoints_x;
	delete [] inpoints_y;

	return res;
}

void Interpolation::interpolate (InstanceGroup *ig, const set<string> &counters,
	const string & TimeUnit)
{
	map<string, InterpolationResults*> res;
	vector<Instance *> i = ig->getInstances();
	if (i.size() > 0)
	{
		set<string>::iterator it;

		map<string, double> SlopeFactors;
		map<string, double> AvgCounters;
		for (it = counters.begin(); it != counters.end(); it++)
		{
			SlopeFactors[*it] = 0;

			unsigned long long totDuration = 0;
			unsigned long long totCounter = 0;
			unsigned count = 0;
			for (unsigned u = 0; u < i.size(); u++)
				if (i[u]->hasCounter (*it))
				{
					/* If we have any instance, calculate its slope factor by
					   calculating totalDuration. For time, divide by 1000 as
					   time is ns so everything gets in Mevents */
					if (TimeUnit == common::DefaultTimeUnit)
						totDuration += i[u]->getDuration() / 1000;
					else
						totDuration += i[u]->getTotalCounterValue (TimeUnit);
					totCounter += i[u]->getTotalCounterValue(*it);
					count++;
				}

			if (count > 0)
			{
				SlopeFactors[*it] = ((double) totCounter) / ((double) totDuration);
				AvgCounters[*it] = ((double) totCounter) / ((double) count);
			}
		}

		map<string, vector<Sample*> > mvs = ig->getSamples();
		for (it = counters.begin(); it != counters.end(); it++)
		{
			if (mvs.count(*it) > 0)
			{
				vector<Sample*> vs = mvs[*it];
				InterpolationResults *ir = interpolate_kernel (vs, *it,
				  ig->getRegionName(), ig->getNumGroup());
				if (common::isMIPS (*it))
					ig->setInterpolationBreakpoints (ir->getBreakpoints());
				ir->setAvgCounterValue (AvgCounters[*it]);
				ir->calculateSlope (SlopeFactors[*it]);
				res.insert (pair<string, InterpolationResults*> (*it, ir));
			}
		}
	}

	ig->setInterpolated (res);
}

