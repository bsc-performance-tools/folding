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

static char __attribute__ ((unused)) rcsid[] = "$Id$";

#include "common.H"

#include <algorithm>
#include <math.h>

#include "sample-selector-distance.H"

SampleSelectorDistance::SampleSelectorDistance (void) : limitset(false), limit(0)
{
}

SampleSelectorDistance::SampleSelectorDistance (unsigned limit) : limitset(true), limit(limit)
{
}

void SampleSelectorDistance::Select (InstanceGroup *ig, set<string> &counters)
{
	map<string, vector<Sample*> > used_res, unused_res;

	set<string>::iterator c;

	for (c = counters.begin(); c != counters.end(); c++)
	{
		vector<Instance*> vi = ig->getInstances();
		vector<Sample*> used, unused;
		unsigned cnt = 0;

		for (unsigned i = 0; i < vi.size(); i++)
			if (vi[i]->hasCounter(*c))
			{
				vector<Sample*> vs = vi[i]->getSamples();
				for (unsigned s = 0; s < vs.size(); s++)
					if (vs[s].hasCounter(*c))
						cnt++;
			}

		if (limitset && cnt > limit)
		{
			vector<Sample*> tmp;

			/* tmp will contain all the samples with the c counter */
			for (unsigned i = 0; i < vi.size(); i++)
				if (vi[i]->hasCounter(*c))
				{
					vector<Sample*> vs = vi[i]->getSamples();
					for (unsigned s = 0; s < vs.size(); s++)
						if (vs[s].hasCouner(*c))
							tmp.push_back (vs[s]);
				}

			for (unsigned step = 0; step < limit; step++)
			{
				double d_limit = limit;
				double d_step = step;
				double centerposition = d_step * (1.0 / d_limit) + (1.0 / (2*d_limit) );

				if (tmp.size() > 0)
				{
					vector<Sample*>::iterator it = tmp.begin();
					vector<Sample*>::iterator bestposition = it;
					double distancetobest = fabs((*it)->getNTime() - centerposition); 

					for ( ; it != tmp.end(); it++ )
						if ((*it)->hasCounter (*c) && 
						    fabs ((*it)->getNTime() - centerposition) < distancetobest)
						{
							distancetobest = fabs ((*it)->getNTime() - centerposition);
							bestposition = it;
						}

					used.push_back (*bestposition);
					tmp.erase (bestposition);
				}
			}
	
			/* All the remaining samples in tmp, should go to unused */
			for (unsigned i = 0; i < tmp.size(); i++)
				unused.push_back (tmp[i]);
		}
		else
		{
			/* We don't reach the limit, just set all the samples with the counter into used */
			for (unsigned i = 0; i < vi.size(); i++)
			{
				if (vi[i]->hasCounter(*c))
				{
					vector<Sample*> vs = vi[i]->getSamples();
					for (unsigned s = 0; s < vs.size(); s++)
						used.push_back (vs[s]);
				}
			}
		}

		used_res[*c] = used;
		unused_res[*c] = unused;
	}

	ig->setSamples (used_res, unused_res);
}

