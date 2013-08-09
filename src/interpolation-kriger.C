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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/callstackanalysis.C $
 | 
 | @last_commit: $Date: 2013-05-24 16:08:28 +0200 (dv, 24 mai 2013) $
 | @version:     $Revision: 1764 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id: callstackanalysis.C 1764 2013-05-24 14:08:28Z harald $";

#include "common.H"

#include <iostream>
#include <sstream>
#include <iomanip>

#include <math.h>

#include "interpolation-kriger.H"
#include "kriger_wrapper.h"
#include "sample-selector-distance.H"

InterpolationKriger::InterpolationKriger (unsigned steps, double nuget, 
	bool prefilter) 
	: Interpolation(steps, prefilter)
{
	this->nuget = nuget;
}

string InterpolationKriger::details (void) const
{
	stringstream ss;
	ss << setprecision(1) << scientific << nuget;
	return string ("Kriger (nuget=") + ss.str() + ")";
}

double InterpolationKriger::Distance_Point_To_Interpolate (double inpoint_x,
	double inpoint_y, int outcount, double *outpoints)
{
	double min_distance = 1.0f;
	for (int i = 0; i < outcount; i++)
	{
		double X_distance = inpoint_x-((double) i/(double) outcount);
		double Y_distance = inpoint_y-outpoints[i];
		double distance = sqrt (X_distance*X_distance + Y_distance*Y_distance);

		min_distance = MIN(distance, min_distance);
	}

	return min_distance;
}

unsigned InterpolationKriger::do_interpolate (unsigned inpoints, double *ix,
	double *iy, InterpolationResults *ir, string counter, string region,
	unsigned group)
{
	UNREFERENCED(group);
	UNREFERENCED(counter);
	UNREFERENCED(region);

	unsigned o_count = ir->getCount();
	double *o_ptr = ir->getInterpolationResultsPtr();

	Kriger_Region (inpoints, ix, iy, o_count, o_ptr, 0.0f, 1.0f, nuget);

	vector<double> breakpoints;
	breakpoints.push_back (0.0f);
	breakpoints.push_back (1.0f);
	ir->setBreakpoints (breakpoints);
}

void InterpolationKriger::pre_interpolate (double sigmaTimes, InstanceGroup *ig,
	set<string> &counters)
{
	SampleSelectorDistance ssd;
	ssd.configure (1000);	/* Get up to 1000 samples for this IG */
	ssd.Select (ig, counters);

	vector<Instance *> vi = ig->getInstances();
	if (vi.size() > 0)
	{
		set<string>::iterator it;
		map<string, vector<Sample*> > mvs = ig->getSamples();

		for (it = counters.begin(); it != counters.end(); it++)
		{
			vector<Sample*> vs = mvs[*it];
			InterpolationResults *ir = interpolate_kernel (vs, *it, ig->getRegion(),
			  ig->getNumGroup());

			vector<double> distances;
			vector<Instance *> instances;

			for (unsigned i = 0; i < vi.size(); i++)
				if (vi[i]->hasCounter (*it))
				{
					double avgDistance = 0;
					vector<Sample*> vs = vi[i]->getSamples();
					for (unsigned s = 0; s < vs.size(); s++)
					{
						Sample *sample = vs[s];

						avgDistance += Distance_Point_To_Interpolate (
						  sample->getNTime(),
						  sample->getNCounterValue(*it),
						  steps,
						  ir->getInterpolationResultsPtr());
					}
					if (vs.size() > 1)
						avgDistance = avgDistance / vi[i]->getNumSamples();

					distances.push_back (avgDistance);
					instances.push_back (vi[i]);
				}

			double avgDistance = 0, sigmaDistance = 0;
			if (distances.size() > 0)
			{
				for (unsigned d = 0; d < distances.size(); d++)
					avgDistance += distances[d];
				avgDistance = avgDistance / distances.size();

				if (distances.size() > 1)
					for (unsigned d = 0; d < distances.size(); d++)
						sigmaDistance += (distances[d]-avgDistance)*(distances[d]-avgDistance);
				sigmaDistance = sqrt (sigmaDistance / (distances.size()-1));
			}

			/* If the instance distance to mean is above or below the threshold, remove it */
			unsigned count_excluded = 0;
			for (unsigned d = 0; d < distances.size(); d++)
				if (distances[d] < avgDistance-sigmaTimes*sigmaDistance || 
				    distances[d] > avgDistance+sigmaTimes*sigmaDistance)
				{
					count_excluded++;
					ig->moveToExcluded (instances[d]);
				}

			instances.clear();
			distances.clear();
			delete ir;

			cout << "   " << count_excluded << " out of " << vi.size() << " instances were excluded due to their distance to a reference interpolation" << endl;
		}
	}
}

