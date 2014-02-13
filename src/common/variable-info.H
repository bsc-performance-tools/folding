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

#ifndef VARIABLE_INFO_H_INCLUDED
#define VARIABLE_INFO_H_INCLUDED

#include "common.H"

#include <string>

using namespace std;

class VariableInfo
{
	private:
	const string name;
	const unsigned long long startaddress;
	const unsigned long long endaddress;

	public:
	VariableInfo (const string &n, unsigned long long start,
	  unsigned long long end);
	~VariableInfo();
	unsigned long long getStartAddress (void) const
	  { return startaddress; }
	unsigned long long getEndAddress (void) const
	  { return endaddress; }
	unsigned long long getSize (void) const
	  { return (endaddress + 1) - startaddress; }
	string getName(void) const
	  { return name; }
};

#endif /* VARIABLE_INFO_H_INCLUDED */
