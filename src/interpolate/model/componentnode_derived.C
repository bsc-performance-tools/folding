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
#include "componentnode_derived.H"

#include <assert.h>
#include <iostream>

ComponentNode_derived::ComponentNode_derived (Operator o, ComponentNode *c1,
	ComponentNode *c2) : op(o), child1(c1), child2(c2)
{
	assert (op == ADD || op == SUB || op == MUL || op == DIV);
}

ComponentNode_derived::~ComponentNode_derived ()
{
	delete child1;
	delete child2;
}

double ComponentNode_derived::evaluate (map<string,InterpolationResults*> &ir,
	unsigned pos) const
{
	double v1 = child1->evaluate (ir, pos);
	double v2 = child2->evaluate (ir, pos);

	double res = 0;
	switch (op)
	{
		case ADD: res = v1+v2;
		break;
		case SUB: res = v1-v2;
		break;
		case MUL: res = v1*v2;
		break;
		case DIV: res = v1/v2;
		break;
		case NOP:
		break;
	}

	return res;
}

void ComponentNode_derived::show (unsigned depth) const
{
	for (unsigned u = 0; u < depth; u++)
		cout << "  ";

	cout << "DERIVED ";
	switch (op)
	{
		case ADD: cout << "+" << endl;
		break;
		case SUB: cout << "-" << endl;
		break;
		case MUL: cout << "*" << endl;
		break;
		case DIV: cout << "/" << endl;
		break;
		case NOP: cout << "NOP" << endl;
		break;
	}

	child1->show (depth+1);
	child2->show (depth+1);
}

set<string> ComponentNode_derived::requiredCounters (void) const
{
	set<string> res;
	set<string> tmp1 = child1->requiredCounters();
	set<string> tmp2 = child2->requiredCounters();
	res.insert (tmp1.begin(), tmp1.end());
	res.insert (tmp2.begin(), tmp2.end());
	return res;
}

