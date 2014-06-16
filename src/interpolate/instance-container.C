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

#include <math.h>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

#include "instance-container.H"
#include "instance-separator.H"
#include "generate-gnuplot.H"

InstanceContainer::InstanceContainer (string s, InstanceSeparator *iseparator)
{
	regionName = s;
	ngroups = 1;
	is = iseparator;
}

unsigned InstanceContainer::numSamples (void)
{
	unsigned tmp = 0;
	for (unsigned u = 0; u < Instances.size(); u++)
		tmp += Instances[u]->getNumSamples();
	return tmp;
}

void InstanceContainer::splitInGroups (void)
{
	/* calculate number & split data in groups */
	ngroups = is->separateInGroups (this->Instances);

	/* Create instance groups, even if not need to split where in such case 
	   everything goes to group 0 */
	for (unsigned u = 0; u < ngroups; u++)
	{
		InstanceGroup *g = new InstanceGroup (regionName, u, is->nameGroup (u));
		InstanceGroups.push_back (g);
	}

	for (unsigned u = 0; u < Instances.size(); u++)
		if (Instances[u]->getGroup() < ngroups)
		{
			InstanceGroups[Instances[u]->getGroup()]->add (Instances[u]);
		}
		else
		{
			/* This should not happen! */
			cerr << "Invalid group " << Instances[u]->getGroup() << " for instance. This should not happen, proceed with caution!" << endl;
		}
}

void InstanceContainer::dumpGroupData (const ObjectSelection *os,
	const string & prefix, const string & TimeUnit)
{
	string fname = prefix + "." + os->toString(false, "any") + ".groups.csv";

	ofstream odata;
	odata.open (fname.c_str(), std::ofstream::app);

	if (Instances.size() > 0)
	{
		for (unsigned g = 0; g < ngroups; g++)
		{
			vector<Instance*> v = InstanceGroups[g]->getInstances();
			for (unsigned u = 0; u < v.size(); u++)
				odata << "u;" << regionName << ";" << g+1 << ";" << v[u]->getDuration() << endl;
			v = InstanceGroups[g]->getExcludedInstances();
			for (unsigned u = 0; u < v.size(); u++)
				odata << "e;" << regionName << ";" << g+1 << ";" << v[u]->getDuration() << endl;
		}
	}

	odata.close ();
}

void InstanceContainer::gnuplot (const ObjectSelection *os,
	const string & fileprefix, StatisticType_t type)
{
	gnuplotGenerator::gnuplot_groups (this, os, fileprefix, type);
}

void InstanceContainer::python (const ObjectSelection *os,
	const string & prefix, const set <string> & counters)
{
	if (ngroups > 0 && counters.size() > 0)
	{
		string fname = prefix + ".wxfolding";
		bool writeheader = !common::existsFile (fname);

		ofstream f (fname.c_str(), ios::app);
		if (!f.is_open())
		{
			cerr << "Failed to create " << fname << endl;
			return;
		}

		if (writeheader)
		{
			time_t now = time(0);
			f << "# Folding done by " << getenv("USER") << " at " << ctime(&now);
			f << "Object=" << os->toString (false, "any") << endl;
		}

		f << regionName << ";";

		set<string>::iterator i;
		for (i = counters.begin(); i != counters.end(); i++)
			if (i == counters.begin())
				f << *i;
			else
				f << "," << *i;

		f << ";" << ngroups << ";";

		for (unsigned u = 0; u < ngroups; u++)
		{
			string s = InstanceGroups[u]->python();
			if (u == 0)
				f << s;
			else
				f << "," << s;
		}

		f << endl;

		f.close();
	}
}

void InstanceContainer::removePreviousDataFiles (const ObjectSelection *os,
	const string & prefix)
{
	string wxfname = prefix + ".wxfolding";
	if (common::existsFile (wxfname))
		if (unlink (wxfname.c_str()) != 0)
			cerr << "Can't remove previous file '" << wxfname << "'" << endl;

	string fname = prefix + "." + os->toString(false, "any") + ".groups.csv";
	if (common::existsFile (fname))
		if (unlink (fname.c_str()) != 0)
			cerr << "Can't remove previous file '" << fname << "'" << endl;
}

InstanceGroup* InstanceContainer::getInstanceGroup (unsigned g)
{
	assert (g >= 0);
	assert (g < ngroups);

	return InstanceGroups[g];
}

