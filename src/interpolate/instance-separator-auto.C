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

#include "instance-separator-auto.H"

#include <algorithm>


InstanceSeparatorAuto::InstanceSeparatorAuto (bool keepallgroups)
	: InstanceSeparator (keepallgroups)
{
}

static bool InstanceSortDuration (Instance *i1, Instance *i2)
{ return i1->getDuration() < i2->getDuration(); }

unsigned InstanceSeparatorAuto::separateInGroups (vector<Instance*> &vi)
{
	unsigned ngroups;

	/* Less than 8 instances is a very small number to group */
	if (vi.size() >= 8)
	{
		unsigned buckets = MIN(vi.size()/4, 100);

		vector<Instance*> tmp = vi;
		sort (tmp.begin(), tmp.end(), InstanceSortDuration);

		unsigned long long maxduration = tmp[tmp.size()-1]->getDuration();
		unsigned long long minduration = tmp[0]->getDuration();
		bucketsize = (maxduration - minduration) / buckets;

		unsigned long long oldpos = tmp[0]->getDuration();
		unsigned group = 0;
		tmp[0]->setGroup (group);

		for (unsigned u = 1; u < tmp.size(); u++)
		{
			if (tmp[u]->getDuration() > oldpos+bucketsize)
				group++;

			tmp[u]->setGroup (group);
			oldpos = tmp[u]->getDuration();
		}

		ngroups = group+1;
	}
	else
		ngroups = 1;

	if (!keepallgroups && ngroups > 1)
	{
		KeepLeadingGroup (vi, ngroups);
		ngroups = 1;
	}

	return ngroups;
}

string InstanceSeparatorAuto::details (void) const
{
	return string ("Auto / Bucket size ") + common::convertDouble (((double) bucketsize) / 1000000. , 3) + " ms";
}

string InstanceSeparatorAuto::nameGroup (unsigned g) const
{
	return string ("Group_") + common::convertInt (g);
}

