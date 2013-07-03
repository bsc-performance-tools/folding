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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/common.C $
 | 
 | @last_commit: $Date: 2013-05-24 16:08:28 +0200 (dv, 24 mai 2013) $
 | @version:     $Revision: 1764 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id: common.C 1764 2013-05-24 14:08:28Z harald $";

#include "common.H"

#include <math.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include "instance-container.H"

InstanceContainer::InstanceContainer (string s)
{
	bucketsize = 0;
	regionName = s;
	ngroups = 1;
}

void InstanceContainer::add (Instance *i)
{
	Instances.push_back (i);
}

unsigned InstanceContainer::numInstances (void)
{
	return Instances.size();
}

unsigned InstanceContainer::numSamples (void)
{
	unsigned tmp = 0;
	for (unsigned u = 0; u < Instances.size(); u++)
		tmp += Instances[u]->Samples.size();

	return tmp;
}

static bool InstanceSortDuration (Instance *i1, Instance *i2)
{ return i1->duration < i2->duration; }

void InstanceContainer::separateInGroups (void)
{
	/* Less than 3 instances is a very small number to group */
	if (Instances.size() >= 8)
	{
		unsigned buckets = MIN(Instances.size()/4, 100);

		vector<Instance*> tmp = Instances;
		sort (tmp.begin(), tmp.end(), InstanceSortDuration);

		unsigned long long maxduration = tmp[tmp.size()-1]->duration;
		unsigned long long minduration = tmp[0]->duration;
		bucketsize = (maxduration - minduration) / buckets;

		unsigned long long oldpos = tmp[0]->duration;
		unsigned group = 0;
		tmp[0]->group = group;

		for (unsigned u = 1; u < tmp.size(); u++)
		{
			if (tmp[u]->duration > oldpos+bucketsize)
				group++;

			tmp[u]->group = group;
			oldpos = tmp[u]->duration;
		}

		ngroups = group+1;
	}
	else
	{
		ngroups = 1;
	}
}

void InstanceContainer::splitInGroups (bool split)
{
	if (split)
	{
		/* calculate number & split data in groups */
		separateInGroups();
	}

	/* Create instance groups, even if not need to split where in such case 
	   everything goes to group 0 */
	for (unsigned u = 0; u < ngroups; u++)
	{
		InstanceGroup *g = new InstanceGroup (regionName, u);
		InstanceGroups.push_back (g);
	}

	for (unsigned u = 0; u < Instances.size(); u++)
		if (Instances[u]->group < ngroups)
		{
			InstanceGroups[Instances[u]->group]->add (Instances[u]);
		}
		else
		{
			/* This should not happen! */
			cerr << "Invalid group " << Instances[u]->group << " for instance. This should not happen, proceed with caution!" << endl;
		}
}

void InstanceContainer::dumpGroupData (ObjectSelection *os, string prefix)
{
	string fname = prefix + "." + os->toString(false, "any") + ".groups.csv";

	ofstream odata;
	odata.open (fname.c_str(), std::ofstream::app);

	if (Instances.size() > 0)
		for (unsigned u = 0; u < Instances.size(); u++)
			odata << regionName << ";" << Instances[u]->duration << ";" << Instances[u]->group+1 << endl;

	odata.close ();
}

void InstanceContainer::gnuplot (ObjectSelection *os, string prefix)
{
	string fname = prefix + "." + os->toString(false, "any") + ".groups.csv";
	string gname = prefix + "." + regionName + "." + os->toString (false, "any")
	  + ".groups.gnuplot";

	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return;
	}

	gplot << fixed <<
	  "# set term postscript eps enhaced solid color" << endl <<
	  "# set term png size 800,600" << endl <<
	  "set datafile separator \";\"" << endl <<
	  "set key bottom outside center horizontal samplen 1 font \",9\"" << endl <<
	  "set yrange [-1:1];" << endl <<
	  "set noytics; " << endl <<
	  "set xtics nomirror rotate by -90; " << endl <<
	  "set border 1" << endl <<
	  "set xlabel \"Time (ms)\";" << endl <<
	  "set title \"Instance groups for " << os->toString(true) << " - Region " << 
	  regionName << "\\n" << Instances.size() << " Instances - Bucket size " << 
	  fixed << setprecision (3) << ((double) bucketsize) / 1000000. << " ms\";" << endl;

	for (unsigned u = 0; u < ngroups; u++)
	{
		if (u == 0)
			gplot << "plot \\" << endl;
		else
			gplot << ",\\" << endl;
		gplot << "'" << fname << "' using ((strcol(1) eq '" << regionName 
		  << "' && $3 == " << u+1 << ") ? $2 / 1000000: NaN) : $0 ti 'Group " << u+1
		  << " (" << InstanceGroups[u]->numInstances() << ")'";
	}
	gplot << ";" << endl;

	gplot.close();

}

void InstanceContainer::removePreviousData (ObjectSelection *os, string prefix)
{
	string fname = prefix + "." + os->toString(false, "any") + ".groups.csv";

	if (common::existsFile (fname))
		if (unlink (fname.c_str()) != 0)
			cerr << "Can't remove previous file '" << fname << "'" << endl;
}

