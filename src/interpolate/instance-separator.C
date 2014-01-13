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
 | @file: $HeadURL$
 | 
 | @last_commit: $Date$
 | @version:     $Revision$
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id$";

#include "common.H"

#include "instance-separator.H"

InstanceSeparator::InstanceSeparator (bool keepallgroups) :
	keepallgroups (keepallgroups)
{
}

void InstanceSeparator::KeepLeadingGroup (vector<Instance*> &Instances, unsigned ngroups)
{
	vector<unsigned> occurrences (ngroups, 0);

	/* Count how many instances per group */
	vector<Instance*>::iterator it;
	for (it = Instances.begin(); it < Instances.end(); it++)
		occurrences[(*it)->getGroup()]++;

	/* Take the lead group of them */
	unsigned leadinggroup = 0;
	for (unsigned g = 1; g < ngroups; g++)
		if (occurrences[leadinggroup] <= occurrences[g])
			leadinggroup = g;

	/* Those instances that are in the lead, move to group 0, remove the rest */
	it = Instances.begin();
	while (it != Instances.end())
	{
		if ((*it)->getGroup() == leadinggroup)
		{
			(*it)->setGroup (0);
			it++;
		}
		else
			it = Instances.erase (it);
	}
}

