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

#include <iostream>

#include "callstack-codereftriplet.H"

Callstack_CodeRefTriplet::Callstack_CodeRefTriplet (
	map<unsigned,CodeRefTriplet> & crt)
{
	this->crt = crt;
}

void Callstack_CodeRefTriplet::processCodeTriplets (void)
{
	/* Remove head callers 0..2 */
	map<unsigned, CodeRefTriplet>::iterator i = crt.begin();
	if ((*i).second.getCaller() <= 2) /* 0 = End, 1 = Unresolved, 2 = Not found */
	{
		do
		{
			crt.erase (i);
			if (crt.size() == 0)
				break;
			i = crt.begin();
		} while ((*i).second.getCaller() <= 2);
	}

	/* Remove tail callers 0..2 */
	i = crt.begin();
	bool tailFound = false;
	map<unsigned, CodeRefTriplet>::iterator it;
	while (i != crt.end())
	{
		if ((*i).second.getCaller() <= 2 && !tailFound)
		{
			it = i;
			tailFound = true;
		}
		else if ((*i).second.getCaller() > 2)
			tailFound = false;
		i++;
	}
	if (tailFound)
		crt.erase (it, crt.end());
}

bool Callstack_CodeRefTriplet::complete_match (
	Callstack_CodeRefTriplet other) const
{
	/* Check if this object is equal to another one */

	if (getSize() != other.getSize())
		return false;

	const map<unsigned, CodeRefTriplet> & other_map = other.getAsConstReferenceMap ();
	map<unsigned, CodeRefTriplet>::const_reverse_iterator other_it = other_map.crbegin();
	map<unsigned, CodeRefTriplet>::const_reverse_iterator mine_it = crt.crbegin();

	bool match = true;
	while (match &&
	  other_it != other_map.crend() &&
	  mine_it != crt.crend())
	{
		match = (*mine_it).second.getCaller() == (*other_it).second.getCaller();
		other_it ++;
		mine_it ++;
	}

	return match;
}

int Callstack_CodeRefTriplet::prefix_match (
	Callstack_CodeRefTriplet other,
	bool & match) const
{
	/* Check if my callstack partially matches at some level with another */

	const map<unsigned, CodeRefTriplet> & other_map = other.getAsConstReferenceMap ();
	map<unsigned, CodeRefTriplet>::const_reverse_iterator other_it = other_map.crbegin();
	map<unsigned, CodeRefTriplet>::const_reverse_iterator mine_it = crt.crbegin();

	int distance = 0;
	match = false;
	while (mine_it != crt.crend())
	{
		/* If my head (at distance) is the head of the other, we have found it,
		   stop looking */
		if ( (*mine_it).second.getCaller() == (*other_it).second.getCaller() )
			break;

		mine_it++;
		distance++;
	}

	if (mine_it != crt.crend())
		match = true;

	return (*other_it).first - (*mine_it).first;
}

void Callstack_CodeRefTriplet::show (bool oneline) const
{
	map<unsigned, CodeRefTriplet>::const_reverse_iterator it1;
	unsigned last_depth = 0;

	for (it1 = crt.crbegin(); it1 != crt.crend(); it1++)
	{
		cout << "[ " << (*it1).first << " <" << (*it1).second.getCaller() 
		  << "," << (*it1).second.getCallerLine() << "," 
		  << (*it1).second.getCallerLineAST() << "> ]";
		if (!oneline)
			cout << endl;

		last_depth = (*it1).first;
	}

	if (oneline)
		cout << endl;
}

void Callstack_CodeRefTriplet::addBubbles (unsigned nbubbles)
{
	map<unsigned, CodeRefTriplet> new_map;

	for (auto const & unsigned_triplet : crt)
		new_map[unsigned_triplet.first + nbubbles] = unsigned_triplet.second;

	crt = new_map;
}

void Callstack_CodeRefTriplet::setMaxDepth (unsigned max)
{
	CodeRefTriplet empty;

	for (unsigned u = 0; u <= max; u++)
		if (crt.count (u) == 0)
			crt[u] = empty;
}
