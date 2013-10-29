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

#include <iostream>
#include <assert.h>

#include "callstack.H"

Sample * Callstack::lookLongestWithMain (vector<Sample*> &vs, unsigned mainid)
{
	Sample *res = NULL;

	for (unsigned u = 0; u < vs.size(); u++)
	{
		Sample *s = vs[u];
		if (s->hasCaller(mainid))
		{
			if (res != NULL)
			{
				if (s->getCodeRefTripletSize() > res->getCodeRefTripletSize())
					res = s;
			}
			else
				res = s;
		}
	}

	return res;
}

Sample * Callstack::lookLongest (vector<Sample*> &vs)
{
	Sample *res = NULL;

	for (unsigned u = 0; u < vs.size(); u++)
	{
		Sample *s = vs[u];
		map<unsigned, CodeRefTriplet> ct = s->getCodeTriplets();
		map<unsigned, CodeRefTriplet>::iterator i;
		for (i = ct.begin(); i != ct.end(); i++)
			if (res != NULL)
			{
				if (s->getCodeRefTripletSize() > res->getCodeRefTripletSize())
					res = s;
			}
			else
				res = s;
	}

	return res;
}

void Callstack::generate (InstanceGroup *ig, bool hasmain, unsigned mainid)
{
	vector<double> phase_ranges = ig->getInterpolationBreakpoints();
	vector<Instance*> vi = ig->getInstances();
	vector<CallstackTree *> vtree;
	vector< map< unsigned, CodeRefTripletAccounting*> > accPerLine;

	if (common::DEBUG())
		cout << "generateTree for InstanceGroup " << ig << " with main? " << hasmain
		  << " mainid = " << mainid << endl;

	/* Phases must have at least two elements [0.0, 1.0] */
	assert (phase_ranges.size() >= 2);

	/* Phases must start at 0.0 and end at 1.0 */
	assert (phase_ranges[0]==0.0 && phase_ranges[phase_ranges.size()-1]==1.0);

	for (unsigned phase = 0; phase < phase_ranges.size()-1; phase++)
	{
		double delta = phase_ranges[phase+1] - phase_ranges[phase];
		double phase_begin = (phase_ranges[phase] == 0.0f) ? 0.0f : phase_ranges[phase]   + 0.05*delta;
		double phase_end = (phase_ranges[phase+1] == 1.0f) ? 1.0f : phase_ranges[phase+1] - 0.05*delta;

		vector<Sample *> vs;
		for (unsigned i = 0; i < vi.size(); i++)
		{
			vector<Sample*> v = vi[i]->getSamples();
			for (unsigned s = 0; s < v.size(); s++)
				if (v[s]->getNTime() >= phase_begin && v[s]->getNTime() < phase_end)
					vs.push_back (v[s]);
		}

		map<unsigned, CodeRefTripletAccounting*> accountPerLine;

		Sample *sroot;
		if (hasmain)
			sroot = lookLongestWithMain (vs, mainid);
		else
			sroot = lookLongest (vs);

		if (common::DEBUG())
			cout << "sroot = " << sroot << " in phase " << phase << endl;

		if (sroot != NULL)
		{
			/* Generate a new tree using this root (main) and connecting to the superroot */
			CallstackTree *root;
			if (!hasmain)
			{
				/* If we don't have a main, invent it, and join the remaining part */
				root = new CallstackTree ();
				CallstackTree *pseudoroot = new CallstackTree (sroot, 0);
				root->insert (pseudoroot);
			}
			else
				root = new CallstackTree (sroot, 0);

			if (common::DEBUG())
			{
				cout << "Initial root sample & root tree" << endl;
				sroot->show();
				root->show();
			}

			/* Remove the root sample from the copied vector & those
			   samples with no callstack info */
			vector<Sample*>::iterator it = vs.begin();
			while (it != vs.end())
			{
				if (*it == sroot || (*it)->getCodeRefTripletSize() == 0)
					it = vs.erase (it);
				else
					it++;
			}

			/* Duplicate the vector to work with it */
			vector<Sample*> copy = vs;

			unsigned inserted;
			do
			{
				inserted = 0;

				it = copy.begin();
				while (it != copy.end())
				{
					CallstackTree *ct;
					Sample *s = *it;
					unsigned d;

					if (common::DEBUG())
					{
						cout << "Adding sample into callstack" << endl;
						s->show();
					}

					if (!hasmain)
						ct = root->findDeepestCommonCallerWithoutMain (s, d);
					else
						ct = root->findDeepestCommonCaller (s, 0, d);

					if (ct != NULL)
					{
						if (common::DEBUG())
							cout << "Deepest common caller info : depth = " << d 
							  << " sample depth = " << s->getCodeRefTripletSize()
							  << " where in CT = " << ct << endl;

						map<unsigned, CodeRefTriplet> crt = s->getCodeTriplets();
						map<unsigned, CodeRefTriplet>::iterator top = crt.begin();
						CodeRefTriplet codetop = (*top).second;
						if (accountPerLine.count (codetop.getCallerLine()) == 0)
						{
							CodeRefTripletAccounting *crta = new CodeRefTripletAccounting (codetop);
							accountPerLine[codetop.getCallerLine()] = crta;
						}
						else
						{
							CodeRefTripletAccounting *crta = accountPerLine[codetop.getCallerLine()];
							crta->increaseCount();
						}

						if (d != s->getCodeRefTripletSize())
						{
							CallstackTree *child = new CallstackTree (s, d);

							if (common::DEBUG())
							{
								cout << "Inserting into following full tree" << endl;
								root->show();
								cout << "WHERE in tree" << endl;
								ct->show();
								cout << "WHAT to insert in tree" << endl;
								child->show();
							}
							ct->insert (child);
							if (common::DEBUG())
							{
								cout << "Resulting tree" << endl;
								root->show();
							}
						}
						else
							ct->increaseOccurrences ();

						it = copy.erase (it);
						inserted++;
					}
					else
						it++;
				}

				if (common::DEBUG())
				{
					cout << "Tree after adding " << inserted << " samples in step " << endl;
					root->show();
				}

			} while (inserted > 0);

			if (copy.size() > 0)
			{
				Instance *inst = vi[0];
				cout << "Warning! " << copy.size() << " unused samples of "
				  << vs.size() << " to generate the call-tree for Region "
				  << inst->getRegionName() << " Group " << inst->getGroup() + 1 
				  << " Phase " << phase + 1 << endl;

				if (common::DEBUG())
				{
					for (unsigned u = 0; u < copy.size(); u++)
						copy[u]->show();
				}
			}

			if (common::DEBUG())
			{
				cout << "*** Final Tree ***" << endl;
				root->show();
			}

			vtree.push_back (root);
			accPerLine.push_back (accountPerLine);
		}
		else
			vtree.push_back (NULL);
	}

	assert (vtree.size() == phase_ranges.size()-1);
	assert (accPerLine.size() == phase_ranges.size()-1);

	ig->setCallstackTrees (vtree);
	ig->setAccountingPerLine (accPerLine);
}

