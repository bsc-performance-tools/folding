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

#include <stdlib.h>

#include <iostream>
#include <iomanip>

#include "common.H"
#include "callstackanalysis.H"


void ca_callstackanalysis::do_analysis (unsigned R, string Rstr,
	vector<double> &breakpoints, vector<ca_callstacksample> &samples,
	UIParaverTraceConfig *pcf, cube::Cube *cube)
{
	unsigned N = breakpoints.size();

	cout << "CallStack Analysis for " << Rstr << " Num of phases = " << N-1 << endl;

	if (N < 1 || breakpoints[0] != 0.0f)
	{
		cerr << "First breakpoint has to be 0.0f" << endl;
		exit (-1);
	}

	cube::Metric* met0 = cube->def_met("# Occurrences", "no_Occurrences", "INTEGER",
	  "occ", "", "", "Number of occurrences of the callstack", NULL);
	string RegionSTR = Rstr;

  cube::Region* rroot = cube->def_region (RegionSTR, 0, 0, "", "", "");
  cube::Cnode* croot = cube->def_cnode(rroot, "", 0, NULL);

	for (unsigned u = 1; u < N; u++)
	{
		RegionSTR = "Phase " + common::convertInt (u) + " (" + 
		  common::convertDouble (breakpoints[u-1],2) + "-" + 
		  common::convertDouble (breakpoints[u],2) + ")";
	  cube::Region* r = cube->def_region (RegionSTR, 0, 0, "", "", "");
		cube::Cnode* c = cube->def_cnode (r, "", 0, croot);

		do_analysis_presence_region (R, Rstr, u, breakpoints[u-1], breakpoints[u],
		  samples, pcf);

		if (pcf != NULL)
			do_analysis_presence_region_cube_tree (R, Rstr, u, breakpoints[u-1],
			  breakpoints[u], samples, pcf, cube, c);
	}
}

void ca_callstackanalysis::do_analysis_presence_region (unsigned R, string Rstr,
	unsigned phase,	double from, double to, vector<ca_callstacksample> &samples,
	UIParaverTraceConfig *pcf)
{
	vector<ca_callstackanalysis_presence> found_paths;
	unsigned MAXoccurrences = 0;
	unsigned TOTALoccurrences = 0;

	unsigned N = samples.size();
	for (unsigned u = 0; u < N; u++)
	{
		/* (], don't get samples replicated if they're on the edge */
		if (samples[u].Region == R &&
		    samples[u].Time >= from && samples[u].Time < to) 
		{
			/* Check if the callstack is already found */
			unsigned NN = found_paths.size();
			bool found = false;
			unsigned found_position = 0;
			for (unsigned uu = 0; uu < NN && !found; uu++)
			{
				if (found_paths[uu].caller.size() == samples[u].caller.size() &&
				    found_paths[uu].callerline.size() == samples[u].callerline.size())
				{
					bool equal = true;
					unsigned NNN = found_paths[uu].caller.size();
					for (unsigned uuu = 0 ; uuu < NNN && equal; uuu++)
						equal =
							found_paths[uu].caller[uuu] == samples[u].caller[uuu] &&
							found_paths[uu].callerline[uuu] == samples[u].callerline[uuu];

					found = equal;
				}
				if (found)
					found_position = uu;
			}

			if (found)
			{
				found_paths[found_position].occurrences++;

				MAXoccurrences = MAX(found_paths[found_position].occurrences, MAXoccurrences);
			}
			else
			{
				ca_callstackanalysis_presence p;
				p.caller = samples[u].caller;
				p.callerline = samples[u].callerline;
				p.occurrences = 1;
				found_paths.push_back(p);

				MAXoccurrences = MAX(1, MAXoccurrences);
			}
			TOTALoccurrences++;
		}
	}

	if (TOTALoccurrences > 0)
	{
		cout << "Summary for phase ranging from " << from << " to " << to << " (total occurrences = " << TOTALoccurrences << ", paths = " << found_paths.size() << ")" << endl;

		unsigned N = found_paths.size();
		for (unsigned u = 0; u < N; u++)
		{
			//if (100*found_paths[u].occurrences >= 5*TOTALoccurrences) /* show only above 5% */
			{
				double d_occurrences = (double) found_paths[u].occurrences;
				double d_total = (double) TOTALoccurrences;

				cout << fixed << setprecision (2) << "% of occurrences " << d_occurrences/d_total*100 << " CALLSTACK: ";
				unsigned NN = found_paths[u].caller.size();
				for (unsigned uu = 0; uu < NN; uu++)
					if (pcf != NULL)
						cout << "[" << pcf->getEventValue (30000000, found_paths[u].caller[uu]) << "-" << pcf->getEventValue (30000100,found_paths[u].callerline[uu]) << "]";
					else
						cout << "[" << found_paths[u].caller[uu] << "," << found_paths[u].callerline[uu] << "]";
				cout << endl;
			}
		}
	}
}

void ca_callstackanalysis::do_analysis_presence_region_cube_tree_r (TreeNodeHolder *tree,
	UIParaverTraceConfig *pcf, cube::Cube *cube, cube::Cnode *root)
{
	cube::Metric *m = cube->get_met ("no_Occurrences");
	cube::Thread *t = (cube->get_thrdv())[0];
	for (unsigned u = 0; u < tree->children.size(); u++)
	{
		TreeNodeHolder *child = tree->children[u];
			string s = pcf->getEventValue (30000000, child->Caller) + " - " + pcf->getEventValue (30000100,child->CallerLine);

		cube::Region* r = cube->def_region (s, 0, 0, "", "", "");
		cube::Cnode* c = cube->def_cnode (r, "", 0, root);
		cube->set_sev (m, c, t, child->Occurrences);

		do_analysis_presence_region_cube_tree_r (child, pcf, cube, c);
	}
}

void ca_callstackanalysis::do_analysis_presence_region_cube_tree (unsigned R,
	string Rstr,
	unsigned phase,	double from, double to, vector<ca_callstacksample> &samples,
	UIParaverTraceConfig *pcf, cube::Cube *cube, cube::Cnode *root)
{
	unsigned TOTALoccurrences = 0;
	TreeNodeHolder tnh;
	tnh.Caller = 1;
	tnh.CallerLine = 1;

	for (unsigned u = 0; u < samples.size(); u++)
	{
		/* (], don't get samples replicated if they're on the edge */
		if (samples[u].Region == R &&
		    samples[u].Time >= from && samples[u].Time < to) 
		{
			TreeNodeHolder *node = tnh.lookForCallerLine (samples[u].callerline[0]);
			if (node == NULL)
				node = &tnh;
			node->AddPath (0, samples[u]);

			TOTALoccurrences++;
		}
	}

	if (TOTALoccurrences > 0)
	{
		cube::Metric *m = cube->get_met ("no_Occurrences");
		cube::Thread *t = (cube->get_thrdv())[0];
		for (unsigned u = 0; u < tnh.children.size(); u++)
		{
			TreeNodeHolder *child = tnh.children[u];

			string s = pcf->getEventValue (30000000, child->Caller) + " - " + pcf->getEventValue (30000100,child->CallerLine);
			cube::Region* r = cube->def_region (s, 0, 0, "", "", "");
			cube::Cnode* c = cube->def_cnode (r, "", 0, root);
			cube->set_sev (m, c, t, child->Occurrences);

			do_analysis_presence_region_cube_tree_r (child, pcf, cube, c);
		}
	}
}

