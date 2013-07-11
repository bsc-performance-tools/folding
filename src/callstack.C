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
#include <assert.h>

#include "callstack.H"

bool Callstack::hasMain (Sample* s, unsigned mainid)
{
	map<unsigned, CodeRefTriplet>::iterator i = s->CodeTriplet.begin();
	for (; i != s->CodeTriplet.end(); i++)
		if ((*i).second.Caller == mainid)
			return true;

	return false;
}

Sample * Callstack::lookLongestWithMain (vector<Sample*> &vs, unsigned mainid)
{
	Sample *res = NULL;
	bool found = false;
	unsigned longitude;

	for (unsigned u = 0; u < vs.size(); u++)
	{
		Sample *s = vs[u];
		map<unsigned, CodeRefTriplet>::iterator i = s->CodeTriplet.begin();
		for (; i != s->CodeTriplet.end(); i++)
		{
			if ((*i).second.Caller == mainid)
			{
				if (!found)
				{
					res = s;
					longitude = s->CodeTriplet.size();
					found = true;
				}
				else
				{
					if (s->CodeTriplet.size() > longitude)
						res = s;
				}
			}
		}
	}

	return res;
}

void Callstack::generate (InstanceGroup *ig, unsigned mainid)
{
	Sample *sroot = NULL;
	vector<double> phase_ranges = ig->getInterpolationPhases();
	vector<Instance*> vi = ig->getInstances();
	vector<Sample *> vs;
	vector<CallstackTree *> vtree;

	if (common::DEBUG())
		cout << "generateTree for InstanceGroup " << ig << " mainid = " << mainid << endl;

	/* Phases must have at least two elements [0.0, 1.0] */
	assert (phase_ranges.size() >= 2);

	/* Phases must start at 0.0 and end at 1.0 */
	assert (phase_ranges[0]==0.0 && phase_ranges[phase_ranges.size()-1]==1.0);

	for (unsigned phase = 0; phase < phase_ranges.size() - 1; phase++)
	{
		double phase_begin = phase_ranges[phase];
		double phase_end = phase_ranges[phase+1];

		for (unsigned i = 0; i < vi.size(); i++)
			for (unsigned s = 0; s < vi[i]->Samples.size(); s++)
				if (vi[i]->Samples[s]->nTime >= phase_begin && 
				    vi[i]->Samples[s]->nTime < phase_end)
					vs.push_back (vi[i]->Samples[s]);

		Sample *sroot = lookLongestWithMain (vs, mainid);

		if (common::DEBUG())
			cout << "sroot = " << sroot << " in phase " << phase << endl;

		if (sroot != NULL)
		{
			/* Generate superroot node */
			CallstackTree *superroot = new CallstackTree;

			/* Generate a new tree using this root (main) and connecting to the superroot */

			CallstackTree *root = new CallstackTree (sroot, 0, superroot);
			if (common::DEBUG())
			{
				cout << "Tree after adding root (with mainid = " << mainid << ")" << endl;
				superroot->show();
			}

			/* Remove the root sample from the copied vector & those
			   samples with no callstack info */
			vector<Sample*>::iterator it = vs.begin();
			while (it != vs.end())
			{
				if (*it == sroot || (*it)->CodeTriplet.size() == 0)
					it = vs.erase (it);
				else
					it++;
			}

			/* Duplicate the vector to work with it */
			vector<Sample*> copy = vs;

			/* Try to add all samples one by one */
			unsigned inserted;
			do
			{
				CallstackTree *ct;
				inserted = 0;

				it = copy.begin();
				while (it != copy.end())
				{
					if ((ct = superroot->findTopCaller (*it)) != NULL)
					{
						ct->addCommon (*it);
						inserted++;
						it = copy.erase (it);
					}
					else
						it++;
				}

				if (common::DEBUG())
				{
					cout << "Tree after processing sample " << endl;
					superroot->show();
				}

			} while (inserted > 0 && copy.size () > 0);

			if (copy.size() > 0)
			{
				Instance *inst = vi[0];
				cout << "Warning! " << copy.size() << " unused samples of "
				  << vs.size() << " to generate the call-tree for region "
				  << inst->RegionName << " group " << inst->group + 1 << endl;

				if (common::DEBUG())
				{
					for (unsigned u = 0; u < copy.size(); u++)
						copy[u]->show();
				}
			}

			superroot->sort();

			if (common::DEBUG())
			{
				cout << "Final Tree" << endl;
				superroot->show();
			}

			vtree.push_back (superroot);
		}
		else
		{
			vtree.push_back (NULL);
		}
	}

	assert (vtree.size() == phase_ranges.size()-1);
	ig->setCallstackTrees (vtree);
}

