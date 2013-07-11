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

CallstackTree::CallstackTree (Sample *s, unsigned depth, CallstackTree *parent)
{
	/* Look for the depth-level of this sample, and get the node info */
	map<unsigned, CodeRefTriplet>::reverse_iterator i = s->CodeTriplet.rbegin();
	for (unsigned u = 0; u < depth; u++)
		i++;

	this->nodeTriplet = (*i).second;
	this->parent = parent;

	if (parent != NULL)
		parent->children.push_back (this);

	if (common::DEBUG())
		cout << "Creating Tree Node (" << nodeTriplet.Caller << ", " << 
		  nodeTriplet.CallerLine << ", " << nodeTriplet.CallerLineAST << ")"
		  << endl;

	/* Recursively create the children for this sample */
	if (depth+1 < s->CodeTriplet.size())
		CallstackTree *tmp = new CallstackTree (s, depth+1, this);

	occurrences = 1; /* At least, it's there once! */
}

CallstackTree::CallstackTree (CodeRefTriplet triplet, CallstackTree *parent)
{
	this->nodeTriplet = triplet;
	this->parent = parent;

	if (parent != NULL)
		parent->children.push_back (this);

	if (common::DEBUG())
		cout << "Creating Tree Node (" << nodeTriplet.Caller << ", " << 
		  nodeTriplet.CallerLine << ", " << nodeTriplet.CallerLineAST << ")"
		  << endl;

	occurrences = 1; /* At least, it's there once! */
}

CallstackTree::CallstackTree (void)
{
	if (common::DEBUG())
		cout << "Creating super-root node" << endl;

	this->parent = NULL;
	this->nodeTriplet.Caller = this->nodeTriplet.CallerLine = this->nodeTriplet.CallerLineAST = 0;
	occurrences = 0;
}

CallstackTree * CallstackTree::findTopCaller_r (CodeRefTriplet &ref)
{
	if (common::DEBUG())
		cout << "FindTopCaller comparing " << nodeTriplet.Caller << " with " 
		  << ref.Caller << endl;

	if (nodeTriplet.Caller != ref.Caller)
	{
		for (unsigned c = 0; c < children.size(); c++)
		{
			CallstackTree *foundNode = children[c]->findTopCaller_r (ref);
			if (foundNode != NULL)
				return foundNode;
		}
		return NULL;
	}
	else
		return this;
}

CallstackTree * CallstackTree::findTopCaller (Sample *s)
{
	/* Look for the top-most common point */
	map<unsigned, CodeRefTriplet>::reverse_iterator i = s->CodeTriplet.rbegin();

	return findTopCaller_r ((*i).second);
}

void CallstackTree::addCommon (Sample *s, unsigned sampledepth)
{
	if (sampledepth == 0 && common::DEBUG())
	{
		cout << "Adding Tree Sample " << endl;
		s->show();
	}

	map<unsigned, CodeRefTriplet>::reverse_iterator i = s->CodeTriplet.rbegin();
	for (unsigned u = 0; u < sampledepth; u++)
		i++;

	if (sampledepth == s->CodeTriplet.size() - 1)
	{
		if ((*i).second.CallerLineAST != this->nodeTriplet.CallerLineAST)
		{
			/* If this is the top of the sample callstack, let join the childrens */
			if (common::DEBUG())
				cout << "Joining sample into children to parent " << parent << endl;
	
			assert (parent != NULL);
	
			/* Create a node for this node */
			CallstackTree *tmp = new CallstackTree ((*i).second, parent);
		}
		else
		{
			occurrences++;
		}
	}
	else
	{
		/* Ensure that all the sample is the same <caller,callerline,callerlineast> */
		assert ((*i).second == this->nodeTriplet);
		occurrences++;
	}

	/* Process sample children */
	i++;
	if (i != s->CodeTriplet.rend())
	{
		map<unsigned, CodeRefTriplet>::reverse_iterator tmp = i;
		tmp++;
		bool isLast = tmp == s->CodeTriplet.rend();

		CodeRefTriplet sampleTriplet = (*i).second;
		unsigned u = 0;
		bool found = false;
		for (; u < children.size(); u++)
		{
			if (!isLast)
			{
				if (common::DEBUG())
					cout << "Looking for non-leaf children caller " << children[u]->nodeTriplet.Caller
					  << " comparing to caller " <<  sampleTriplet.Caller<< endl;
				found = children[u]->nodeTriplet.Caller== sampleTriplet.Caller;
				if (found)
					break;
			}
			else
			{
				if (common::DEBUG())
					cout << "Looking for leaf children ast " << children[u]->nodeTriplet.CallerLineAST
					  << " comparing to ast " <<  sampleTriplet.CallerLineAST << endl;
				found = children[u]->nodeTriplet.CallerLineAST == sampleTriplet.CallerLineAST;
				if (found)
					break;
			}
		}
		if (common::DEBUG())
			cout << "Found? = " << found << endl;

		if (!found)
			CallstackTree *tmp = new CallstackTree (s, sampledepth+1, this);
		else
			children[u]->addCommon (s, sampledepth+1);
	}
}

void CallstackTree::show (unsigned depth)
{
	for (unsigned i = 0; i < depth; i++)
		cout << "  ";

	cout << "Triplet (" << nodeTriplet.Caller << ", " << nodeTriplet.CallerLine
	  << ", " << nodeTriplet.CallerLineAST << ") - " << occurrences << endl;

	for (unsigned i = 0; i < children.size(); i++)
		children[i]->show (depth+1);
}

bool CallstackTree_sort_cmp (CallstackTree *p1, CallstackTree *p2)
{
	return p1->getNodeTriplet() < p2->getNodeTriplet();
}

void CallstackTree::sort (void)
{
	std::sort (children.begin(), children.end(), CallstackTree_sort_cmp);

	for (unsigned i = 0; i < children.size(); i++)
		children[i]->sort ();
}

