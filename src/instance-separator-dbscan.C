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

#include "libClustering.hpp"
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE
#undef VERSION

#include "common.H"
#include "instance-separator-dbscan.H"

#include <iostream>
#include <assert.h>

InstanceSeparatorDBSCAN::InstanceSeparatorDBSCAN (unsigned minpoints, 
	double epsilon, bool keepallgroups)
	: InstanceSeparator (keepallgroups), eps(epsilon), minpoints(minpoints)
{
}

unsigned InstanceSeparatorDBSCAN::separateInGroups (vector<Instance*> &vi)
{
	unsigned long long min = 0, max = 0;

	libClustering *c = new libClustering;
	vector<Point*> vp;

	for (unsigned u = 0; u < vi.size(); u++)
	{
		double d = vi[u]->getDuration();
		vector<double> ds;
		ds.push_back (d);
		Point *p = new Point (ds);
		vp.push_back (p);

		if (min != 0 || max != 0)
		{
			if (vi[u]->getDuration() > max)
				max = vi[u]->getDuration();
			else if (vi[u]->getDuration() < min)
				min = vi[u]->getDuration();
		}
		else
			min = max = vi[u]->getDuration();
	}

	vector<const Point*> vcp;
	vector<double> MaxV, MinV, F;
	MaxV.push_back (max);
	MinV.push_back (min);
	F.push_back (1.0f);
	for (unsigned u = 0; u < vp.size(); u++)
	{
		vp[u]->RangeNormalization (MaxV, MinV, F);
		vcp.push_back ((const Point*) vp[u]);
	}

	map<string,string> params;
	params["min_points"] = common::convertInt (minpoints);
	params["epsilon"] = common::convertDouble (eps, 3);
    if (!c->InitClustering("DBSCAN", params))
	{
		cerr << "Failed to init clustering" << endl;
		exit (-1);
	}
	Partition partition;
    if (!c->ExecuteClustering (vcp, partition))
	{
		cerr << "Failed to execute clustering" << endl;
		exit (-1);
	}

	delete c;

	vector<cluster_id_t> clusters = partition.GetAssignmentVector();
	assert (clusters.size() == vcp.size());
	assert (clusters.size() ==  vi.size());

	/* Calculate num groups and associate each instance to its group */
	unsigned ngroups = 0;
	for (unsigned u = 0; u < clusters.size(); u++)
	{
		if (clusters[u] > ngroups)
			ngroups = clusters[u];
		vi[u]->setGroup (clusters[u]);
	}
	ngroups++;

	if (!keepallgroups && ngroups > 1)
	{
		KeepLeadingGroup (vi, ngroups);
		ngroups = 1;
	}

	return ngroups;
}

string InstanceSeparatorDBSCAN::details(void) const
{
	return string("DBSCAN / MinPoints = ") + common::convertInt (minpoints) + " eps = " + common::convertDouble (eps, 3);
}

string InstanceSeparatorDBSCAN::nameGroup (unsigned g) const
{
	if (keepallgroups)
	{
		if (g == 0)
			return "Noise";
		else
			return string ("Group ") + common::convertInt (g);
	}
	else
		return string ("Group ") + common::convertInt (g+1);
}
