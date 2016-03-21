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
#ifndef INSTANCE_SEPARATOR_H_INCLUDED
#define INSTANCE_SEPARATOR_H_INCLUDED

#include <vector>
#include <string>
#include "instance.H"

using namespace std;

/* This is an abstract class for instance separators. Instance separators allow
   to select some instances according to their characteristics. */
class InstanceSeparator
{
	protected:
	const bool keepallgroups;

	public:
	InstanceSeparator (bool keepallgroups);

	/* Main function. Separates the given instances according to the
	   parametrization of the subclass */
	virtual unsigned separateInGroups (vector<Instance*> &vi) = 0;

	/* Provides some details into a string for debugging purposes */
	virtual string details(void) const = 0;

	/* Provides the name of the subgroup */
	virtual string nameGroup (unsigned) const = 0;

	/* Keeps the selected group as a single group with id 0 */
	void KeepLeadingGroup (vector<Instance*> &Instances, unsigned ngroups);
};

#endif /* INSTANCE_SEPARATOR_H_INCLUDED */
