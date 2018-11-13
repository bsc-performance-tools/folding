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
#include "componentnode_alias.H"
#include <assert.h>
#include <iostream>

ComponentNode_alias::ComponentNode_alias (ComponentModel *alias) : alias(alias)
{
  assert(alias != NULL);
}

ComponentNode_alias::~ComponentNode_alias ()
{
}

double ComponentNode_alias::evaluate (map<string,InterpolationResults*> &ir,
	unsigned pos) const
{
	return alias->getComponentNode()->evaluate(ir, pos);;
}

void ComponentNode_alias::show (unsigned depth) const
{
	for (unsigned u = 0; u < depth; u++)
		cout << "  ";
	cout << "ALIAS - " << alias->getName() << endl;
}

set<string> ComponentNode_alias::requiredCounters (void) const
{
	return alias->getComponentNode()->requiredCounters();
}

