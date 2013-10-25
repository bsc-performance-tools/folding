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

#include <math.h>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include "instance-container.H"
#include "instance-separator.H"

InstanceContainer::InstanceContainer (string s, InstanceSeparator *iseparator)
{
	regionName = s;
	ngroups = 1;
	is = iseparator;
}

void InstanceContainer::add (Instance *i)
{
	Instances.push_back (i);
}

unsigned InstanceContainer::numInstances (void) const
{
	return Instances.size();
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

void InstanceContainer::dumpGroupData (ObjectSelection *os, string prefix)
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

void InstanceContainer::gnuplot (ObjectSelection *os, string prefix, StatisticType_t type)
{
	string fname = prefix + "." + os->toString(false, "any") + ".groups.csv";
	string gname = prefix + "." + regionName + "." + os->toString (false, "any")
	  + ".groups.gnuplot";
	vector<string> lightcolors, darkcolors;
	lightcolors.push_back ("#FF0000"); darkcolors.push_back ("#FF8080");
	lightcolors.push_back ("#00B000"); darkcolors.push_back ("#80B080");
	lightcolors.push_back ("#0000FF"); darkcolors.push_back ("#8080FF");
	lightcolors.push_back ("#FF8000"); darkcolors.push_back ("#FF8080");
	lightcolors.push_back ("#FF0080"); darkcolors.push_back ("#FF8080");
	lightcolors.push_back ("#00A0A0"); darkcolors.push_back ("#80A0A0");

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
	  regionName << "\\n" << Instances.size() << " Instances - " << is->details() <<
	  endl;

#define MAXLINECOLOR 6
	gplot << endl
	  << "set style line 1 pt 1 lc rgb \"" << lightcolors[0] << "\"" << endl
	  << "set style line 2 pt 2 lc rgb \"" << lightcolors[1] << "\"" << endl
	  << "set style line 3 pt 3 lc rgb \"" << lightcolors[2] << "\"" << endl
	  << "set style line 4 pt 4 lc rgb \"" << lightcolors[3] << "\"" << endl
	  << "set style line 5 pt 5 lc rgb \"" << lightcolors[4] << "\"" << endl
	  << "set style line 6 pt 6 lc rgb \"" << lightcolors[5] <<  "\"" << endl
	  << endl 
	  << "set style line 7 pt 1 lc rgb \"" << darkcolors[0] << "\"" << endl
	  << "set style line 8 pt 2 lc rgb \"" << darkcolors[1] <<  "\"" << endl
	  << "set style line 9 pt 3 lc rgb \"" << darkcolors[2] << "\"" << endl
	  << "set style line 10 pt 4 lc rgb \"" << darkcolors[3] << "\"" << endl
	  << "set style line 11 pt 5 lc rgb \"" << darkcolors[4] << "\"" << endl
	  << "set style line 12 pt 6 lc rgb \"" << darkcolors[5] << "\"" << endl
	  << endl;

	for (unsigned u = 0; u < ngroups; u++)
	{
		unsigned lstyle = (u % MAXLINECOLOR)+1;
		InstanceGroup *ig = InstanceGroups[u];
		unsigned long long s = (type == STATISTIC_MEAN) ? ig->mean() : ig->median();
		gplot << "set label \"\" at " << s << ".0/1000000.0,0 point lt " << lstyle << " ps 2 lc rgb \"" << lightcolors[lstyle-1] << "\";" << endl;
	}
	gplot << endl;

	gplot << "plot \\" << endl;
	for (unsigned u = 0; u < ngroups; u++)
	{
		unsigned lstyle = (u % MAXLINECOLOR)+1;
		InstanceGroup *ig = InstanceGroups[u];

		if (u != 0)
			gplot << ",\\" << endl;

		gplot << "'" << fname << "' using (strcol(1) eq 'u' && (strcol(2) eq '" 
		  << regionName << "' && $3 == " << u+1
		  << ") ? $4 / 1000000.0: NaN) : $0 ti '" << is->nameGroup (u)
		  << " (" << ig->numInstances() << "/" << ig->numExcludedInstances() 
		  << ")' w points ls " << lstyle << ",\\" << endl;
		gplot << "'" << fname << "' using ((strcol(1) eq 'e') && (strcol(2) eq '" 
		  << regionName 
		  << "' && $3 == " << u+1 << ") ? $4 / 1000000.0: NaN) : $0 notitle w points ls " 
		  << lstyle+MAXLINECOLOR;
	}
	gplot << ";" << endl;

	gplot.close();
}

void InstanceContainer::python (ObjectSelection *os, string prefix,
	set<string> counters)
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

void InstanceContainer::removePreviousDataFiles (ObjectSelection *os, string prefix)
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
