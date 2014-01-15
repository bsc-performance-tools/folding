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

#include "sample-selector-first.H"

SampleSelectorFirst::SampleSelectorFirst (void) : limitset(false), limit(0)
{
}

SampleSelectorFirst::SampleSelectorFirst (unsigned limit) : limitset(true), limit(limit)
{
}

void SampleSelectorFirst::Select (InstanceGroup *ig, const set<string> &counters)
{
	map<string, vector<Sample*> > used_res, unused_res;

	set<string>::iterator c;

	for (c = counters.begin(); c != counters.end(); c++)
	{
		vector<Instance*> vi = ig->getInstances();
		vector<Sample*> used, unused;

		for (unsigned i = 0; i < vi.size(); i++)
			if (vi[i]->hasCounter (*c))
			{
				vector<Sample*> vs = vi[i]->getSamples();
				for (unsigned s = 0; s < vs.size(); s++)
				{
					if (vs[s]->hasCounter(*c))
					{
						if (limitset && used.size() >= limit)
							unused.push_back (vs[s]);
						else
							used.push_back (vs[s]);
					}
					else
						unused.push_back (vs[s]);
				}
			}

		used_res[*c] = used;
		unused_res[*c] = unused;
	}

	ig->setSamples (used_res, unused_res);
}

