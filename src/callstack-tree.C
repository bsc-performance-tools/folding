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

#include "callstack-tree.H"

#include <iostream>
#include <assert.h>
#include <algorithm>

CallstackTree::CallstackTree (Sample *s, unsigned depth)
{
	/* Look for the depth-level of this sample, and get the node info */
	map<unsigned, CodeRefTriplet>::reverse_iterator i = s->CodeTriplet.rbegin();
	for (unsigned u = 0; u < depth; u++)
		i++;

	this->nodeTriplet = (*i).second;

	if (common::DEBUG())
		cout << "Creating Tree Node (" << this->nodeTriplet.Caller << ", " << 
		  this->nodeTriplet.CallerLine << ", " << this->nodeTriplet.CallerLineAST << ")"
		  << endl;

	if (depth+1 < s->CodeTriplet.size())
	{
		CallstackTree *tmp = new CallstackTree (s, depth+1);
		children.push_back (tmp);
		this->occurrences = 0;
	}
	else
		this->occurrences = 1;
}

CallstackTree::CallstackTree (void)
{
	if (common::DEBUG())
		cout << "Creating super-root node" << endl;

	this->nodeTriplet.Caller = this->nodeTriplet.CallerLine = this->nodeTriplet.CallerLineAST = 0;
	this->occurrences = 0;
}

void CallstackTree::insert (CallstackTree *other)
{
	assert (this->nodeTriplet.Caller == other->nodeTriplet.Caller);

	// cout << "inserting " << other->children.size() << " # children " << endl;
	for (unsigned c = 0; c < other->children.size(); c++)
		children.push_back (other->children[c]);
}

CallstackTree* CallstackTree::findDeepestCommonCaller (Sample *s, unsigned depth, unsigned &depth_common)
{
	/* Look for the depth-level of this sample, and get the node info */
	map<unsigned, CodeRefTriplet>::reverse_iterator i = s->CodeTriplet.rbegin();
	for (unsigned u = 0; u < depth; u++)
		i++;
	CodeRefTriplet other = (*i).second;

	if (this->nodeTriplet.Caller == other.Caller)
	{
		// cout << "other.Caller " << other.Caller << " equal" << endl;
		if (depth+1 < s->CodeTriplet.size())
		{
			i++;
			CodeRefTriplet other_child = (*i).second;

			for (unsigned u = 0; u < children.size(); u++)
				if (children[u]->nodeTriplet.Caller == other_child.Caller)
					return children[u]->findDeepestCommonCaller (s, depth+1, depth_common);

			depth_common = depth;
			return this;
		}
		else
		{
			// cout << "at the end of the callstack -> depth_common = " << s->CodeTriplet.size() << endl;
			depth_common = s->CodeTriplet.size();
			return this;
		}
	}
	else
	{
		// cout << "other.Caller " << other.Caller << " different to " << nodeTriplet.Caller << endl;
		return NULL;
	}
}

void CallstackTree::show (unsigned depth)
{
	for (unsigned d = 0; d < depth; d++)
		cout << "  ";
	if (depth > 0)
		cout << "+";
	cout << "Routine " << nodeTriplet.Caller << " #occ = " << occurrences
	  << " @ " << this << endl;

	vector<CallstackTree*>::iterator i;
	for (i = children.begin(); i != children.end(); i++)
		(*i)->show (depth+1);
}

