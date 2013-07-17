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

string InterpolationKriger::details (void)
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

InterpolationResults * InterpolationKriger::interpolate_kernel (vector<Sample*> vs, string counter)
{
	InterpolationResults *res = new InterpolationResults(steps);
	res->setInterpolationDetails (details());

	unsigned incount = 0;
	for (unsigned s = 0; s < vs.size(); s++)
		if (vs[s]->nCounterValue.count(counter) > 0)
			incount++;

	double * inpoints_x = new double [incount+2];
	double * inpoints_y = new double [incount+2];
	double * outpoints = res->getInterpolationResultsPtr();

	bool all_zeroes = true;
	inpoints_x[0] = inpoints_y[0] = 0;
	for (unsigned i = 1, s = 0; s < vs.size(); s++)
		if (vs[s]->nCounterValue.count(counter) > 0)
		{
			inpoints_x[i] = vs[s]->nTime;
			inpoints_y[i] = vs[s]->nCounterValue[counter];
			all_zeroes = all_zeroes && inpoints_y[i] == 0;
			i++;
		}
	inpoints_x[incount+1] = inpoints_y[incount+1] = 1;

	cout << "   - Counter " << counter << ", # in samples = " << incount << endl;
	if (!all_zeroes)
	{
		Kriger_Region (incount+2, inpoints_x, inpoints_y, steps, outpoints, 0.0f, 1.0f, nuget);

		/* Remove values below 0 */
		for (unsigned u = 0; u < steps; u++)
			if (outpoints[u] < 0)
				outpoints[u] = 0;
	}
	else
	{
		for (unsigned u = 0; u < steps; u++)
			outpoints[u] = 0;
	}

	delete [] inpoints_x;
	delete [] inpoints_y;

	return res;
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
			InterpolationResults *ir = interpolate_kernel (vs, *it);

			vector<double> distances;
			vector<Instance *> instances;

			for (unsigned i = 0; i < vi.size(); i++)
			{
				if (vi[i]->Samples[0]->nCounterValue.count (*it) > 0)
				{
					double avgDistance = 0;
					for (unsigned s = 0; s < vi[i]->Samples.size(); s++)
					{
						Sample *sample = vi[i]->Samples[s];

						avgDistance += Distance_Point_To_Interpolate (sample->nTime,
						  sample->nCounterValue[*it], steps,
						  ir->getInterpolationResultsPtr());
					}
					if (vi[i]->Samples.size() > 1)
						avgDistance = avgDistance / vi[i]->Samples.size();

					distances.push_back (avgDistance);
					instances.push_back (vi[i]);
				}
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

void InterpolationKriger::interpolate (InstanceGroup *ig, set<string> &counters)
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
				if (i[u]->TotalCounterValues.count(*it) > 0)
				{
					totDuration += i[u]->duration;
					totCounter += i[u]->TotalCounterValues[*it];
					count++;
				}

			/* If we have any instance, calculate its slope factor 
			   (in Mevents, div 1000, as time is in ns) */
			if (count > 0)
			{
				SlopeFactors[*it] = ((double) totCounter) / ( ((double) totDuration / 1000) );
				AvgCounters[*it] = ((double) totCounter) / ((double) count);
			}
		}

		map<string, vector<Sample*> > mvs = ig->getSamples();
		for (it = counters.begin(); it != counters.end(); it++)
			if (mvs.count(*it) > 0)
			{
				vector<Sample*> vs = mvs[*it];
				InterpolationResults *ir = interpolate_kernel (vs, *it);
				ir->setAvgCounterValue (AvgCounters[*it]);
				ir->calculateSlope (SlopeFactors[*it]);
				res.insert (pair<string, InterpolationResults*> (*it, ir));
			}
	}

	ig->setInterpolated (res);

	vector<double> phases;
	phases.push_back (0.0f);
	phases.push_back (1.0f);
	ig->setInterpolationPhases (phases);
}

