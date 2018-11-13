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
#ifndef COMPONENTNODE_DERIVED_H_INCLUDED
#define COMPONENTNODE_DERIVED_H_INCLUDED

#include "componentnode.H"

class ComponentNode_derived : public ComponentNode
{
	public:
	enum Operator { ADD, SUB, MUL, DIV, MIN, MAX, NOP };

	private:
	const Operator op;
	const ComponentNode *child1, *child2;

	public:
	ComponentNode_derived (Operator o, ComponentNode *c1, ComponentNode *c2);
	~ComponentNode_derived ();

	double evaluate (map<string,InterpolationResults*> &ir, unsigned pos) const;
	void show (unsigned depth) const;
	set<string> requiredCounters (void) const;
};

#endif /* COMPONENTNODE_DERIVED_H_INCLUDED */
