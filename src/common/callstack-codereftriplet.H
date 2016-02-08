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
#ifndef CALLSTACK_CODEREFTRIPLET_H_INCLUDED
#define CALLSTACK_CODEREFTRIPLET_H_INCLUDED

#include <map>
#include "codereftriplet.H"

using namespace std;

class Callstack_CodeRefTriplet
{
	private:
	map<unsigned, CodeRefTriplet> crt;

	public:
	Callstack_CodeRefTriplet (map<unsigned, CodeRefTriplet> & crt);

	void addBubbles (unsigned nbubbles);
	void setMaxDepth (unsigned max);
	void copyBottomStack (const Callstack_CodeRefTriplet other);

	map<unsigned, CodeRefTriplet> getAsMap (void)
	  { return crt; }

	const map<unsigned, CodeRefTriplet> & getAsConstReferenceMap (void) const
	  { return crt; }

	void processCodeTriplets (void);

	unsigned getSize(void) const
	  { return crt.size(); }

	bool complete_match (const Callstack_CodeRefTriplet & other) const;

	int prefix_match (const Callstack_CodeRefTriplet & other, bool & match) const;

	void show (bool endline = false) const;

};

#endif /* CODEREFTRIPLET_H_INCLUDED */

