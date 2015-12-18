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

#include <algorithm>
#include <math.h>
#include <assert.h>

#include "sample-selector-distance-fast.H"

SampleSelectorDistanceFast::SampleSelectorDistanceFast (void)
	: limitset(false), limit(0)
{
}

SampleSelectorDistanceFast::SampleSelectorDistanceFast (unsigned limit)
	: limitset(true), limit(limit)
{
}

static bool TimeSort (const Sample *s1, const Sample *s2)
{
	return s1->getNTime() < s2->getNTime();
}

void SampleSelectorDistanceFast::Select (InstanceGroup *ig,
	const set<string> &counters)
{
	map<string, vector<Sample*> > used_res, unused_res;

	for (auto const & c : counters)
	{
		vector<Sample*> used, unused;
		unsigned cnt = 0;

		for (auto const & i : ig->getInstances())
			if (i->hasCounter(c))
				for (auto const & s : i->getSamples())
					if (s->hasCounter(c))
						cnt++;

		if (limitset && cnt > limit)
		{
			vector<Sample*> tmp;

			/* tmp will contain all the samples with the c counter */
			for (auto const & i : ig->getInstances())
				if (i->hasCounter(c))
				{
					for (auto const & s : i->getSamples())
						if (s->hasCounter(c))
							tmp.push_back (s);
						else
							unused.push_back (s);
				}

			sort (tmp.begin(), tmp.end(), TimeSort);

			assert (tmp.size() >= 2);

			/* Since tmp is sorted by time, search increasingly which is closer
			   to centerposition using two pointers*/
			vector<Sample*>::iterator it = tmp.begin();
			vector<Sample*>::iterator it_next = tmp.begin(); it_next++;

			for (unsigned step = 0; step < limit; step++)
			{
				double d_limit = limit;
				double d_step = step;
				double centerposition = d_step * (1.0 / d_limit) + (1.0 / (2*d_limit) );

				if (it != tmp.end() && it_next != tmp.end())
				{
					if ((*it)->getNTime() < centerposition)
					{
						bool loop = !((*it)->getNTime() < centerposition &&
						              (*it_next)->getNTime() >= centerposition);
						while (loop)
						{
							it_next++; it++;
							if (it_next == tmp.end())
								break;
							loop = !((*it)->getNTime() < centerposition &&
							       (*it_next)->getNTime() >= centerposition);
						}
					}
				}

				if (it_next != tmp.end())
				{
					/* Choose which is closer */
					if (fabs ((*it)->getNTime() - centerposition) <
					    fabs ((*it_next)->getNTime() - centerposition))
					{
						used.push_back (*it);
					}
					else
					{
						used.push_back (*it_next);
					}
					/* Advance to next */
					it++; it_next++;
				}
				else if (it != tmp.end())
				{
					used.push_back (*it);
					it++;
				}
			}

			/* At this point, it is possible that the number of selected samples
			   is not the same as requested samples. Choose remaining samples
			   randomly */
			if (limit > used.size())
			{
				vector<Sample*> tmp2;
				for (const auto s : tmp)
					if (find (used.begin(), used.end(), s) == used.end())
						tmp2.push_back (s);

				/* Choose randomly elements from tmp2 until used reaches 'limit'
				   size. */
				while (limit > used.size())
				{
					long int R = (random())%tmp2.size();
					used.push_back (tmp2[R]);
					tmp2.erase (tmp2.begin()+R);
				}
			}

			/* All the samples not in used, should go into unused */
			for (const auto s : tmp)
				if (find (used.begin(), used.end(), s) == used.end())
					unused.push_back (s);
		}
		else
		{
			/* We don't reach the limit, just set all the samples with the counter into used */
			for (auto const & i : ig->getInstances())
			{
				if (i->hasCounter(c))
				{
					for (auto const & s : i->getSamples())
						if (s->hasCounter (c))
							used.push_back (s);
						else
							unused.push_back (s);
				}
			}
		}

		used_res[c] = used;
		unused_res[c] = unused;
	}

	ig->setSamples (used_res, unused_res);
}

