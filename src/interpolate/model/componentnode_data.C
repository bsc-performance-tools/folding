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
#include "componentnode_data.H"

#include <assert.h>
#include <iostream>

ComponentNode_data::ComponentNode_data (string &ctr) : counter (ctr)
{
}

ComponentNode_data::~ComponentNode_data ()
{
}

double ComponentNode_data::evaluate (map<string,InterpolationResults*> &ir,
	unsigned pos) const
{
	assert (ir.count (counter) > 0);
	return (ir[counter])->getSlopeAt (pos);
}

void ComponentNode_data::show (unsigned depth) const
{
	for (unsigned u = 0; u < depth; u++)
		cout << "  ";
	cout << "DATA - " << counter << endl;
}

set<string> ComponentNode_data::requiredCounters (void) const
{
	set<string> res;
	res.insert (counter);
	return res;
}

