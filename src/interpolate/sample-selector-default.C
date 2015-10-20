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

#include "sample-selector-default.H"

void SampleSelectorDefault::Select (InstanceGroup *ig, const set<string> &counters)
{
	map<string, vector<Sample*> > used_res, unused_res;

	for (auto const & c : counters)
	{
		vector<Sample*> used, unused;
		for (auto const & i : ig->getInstances())
			if (i->hasCounter (c))
			{
				for (auto const & s : i->getSamples())
					if (s->hasCounter(c))
						used.push_back (s);
					else
						unused.push_back (s);
			}
		used_res[c] = used;
		unused_res[c] = unused;
	}

	ig->setSamples (used_res, unused_res);
}

