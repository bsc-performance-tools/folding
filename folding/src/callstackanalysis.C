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

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <fstream>
#include <assert.h>

#include "callstackanalysis.H"

static map<string, cube::Cnode*> KnownRegions;
static map<string, cube::Cnode*> KnownRegionsPhases;
static map<string, TreeNodeHolder*> KnownRegionsPhases_Tree;

static double LookForSlopeAt (string prefix, string counter, string Region, double atposition)
{
	double res = 0;
	string fname = prefix+"."+Region+".slope";
	ifstream f (fname.c_str());
	if (f.is_open())
	{
		string fcounter;
		double time, slope;

		while (f >> fcounter)
		{
			f >> time;
			f >> slope;

			if (time >= atposition && fcounter == counter)
			{
				res = slope;
				break;
			}

		}
		f.close();
	}

	return res;
}


unsigned ca_callstackanalysis::do_analysis (string prefix, string countercode,
	string counter, double value, unsigned R, string Rstr, vector<double> &breakpoints,
	vector<ca_callstacksample> &samples, UIParaverTraceConfig *pcf, cube::Cube *cubev,
	string &sourceDir)
{
	unsigned N = breakpoints.size();

	assert (pcf != NULL);

	if (N < 1 || breakpoints[0] != 0.0f)
	{
		cerr << "First breakpoint has to be 0.0f" << endl;
		return 0; // exit (-1);
	}

	string RegionSTR = Rstr;

	cube::Cnode* croot;
	if (KnownRegions.count(Rstr) == 0)
	{
		cube::Region* rroot = cubev->def_region (RegionSTR, 0, 0, "", "", "");
		croot = cubev->def_cnode(rroot, "", 0, NULL);
		KnownRegions[Rstr] = croot;
	}
	else
		croot = KnownRegions[Rstr];

	for (unsigned u = 1; u < N; u++)
	{
		RegionSTR = "Phase " + common::convertInt (u) + " (" + 
		  common::convertDouble (breakpoints[u-1],2) + "-" + 
		  common::convertDouble (breakpoints[u],2) + ")";

		/* Skip small fraction at the begin and at the end */
		double delta = (breakpoints[u]-breakpoints[u-1]);
		double begin = (breakpoints[u-1] == 0.0f) ? 0.0f : breakpoints[u-1] + 0.05 * delta;
		double end   = (breakpoints[u] == 1.0f) ? 1.0f : breakpoints[u] - 0.05 * delta;

		cube::Cnode *c;
		if (KnownRegionsPhases.count (Rstr+RegionSTR) == 0)
		{
			cube::Region* r = cubev->def_region (RegionSTR, 0, 0, "", "", "");
			c = cubev->def_cnode (r, "", 0, croot);
			KnownRegionsPhases[Rstr+RegionSTR] = c;
		}
		else
			c = KnownRegionsPhases[Rstr+RegionSTR];

#if 0
		do_analysis_presence_region (R, u, breakpoints[u-1], breakpoints[u],
		  samples, pcf);
#endif

#if 0
		cout << "** COUNTER = " << counter << endl;
		cout << "** COUNTER = " << counter << endl;
		cout << "** COUNTER = " << counter << endl;
#endif

		TreeNodeHolder *tnh;
		if (KnownRegionsPhases_Tree.count (Rstr+RegionSTR) == 0)
		{
			tnh = new TreeNodeHolder;
			tnh->Caller = 1;
			tnh->CallerLine = 1;
			tnh->CubeNode = c;

			for (unsigned t = 0; t < samples.size(); t++)
			{
				/* (], don't get samples replicated if they're on the edge */
				if (samples[t].Region == R && samples[t].Time >= begin && samples[t].Time < end) 
				{
#if 0
					TreeNodeHolder *node = tnh.lookForCallerLine (samples[u].callerline[0]);
					if (node == NULL)
						node = &tnh;
#endif
					if (samples[t].callerlineASTBlock.size() > 0)
						tnh->AddPathAST (0, samples[t], pcf, cubev, c, sourceDir);
					else
						tnh->AddPath (0, samples[t], pcf, cubev, c, sourceDir);
				}
			}
			KnownRegionsPhases_Tree[Rstr+RegionSTR] = tnh;
		}
		else
			tnh = KnownRegionsPhases_Tree[Rstr+RegionSTR]; 

		if (counter == "no_Occurrences")
		{
			cube::Metric *m = cubev->get_met (counter);
			do_analysis_presence_region_cube_tree (tnh, pcf, cubev, c, m, sourceDir, counter);
			string s = "Phase";
			spread_constant_tree (tnh, pcf, u, sourceDir, s);
		}
		else if (counter == "duration")
		{
			double severity = (breakpoints[u] - breakpoints[u-1]) * value;
			cube::Metric *m = cubev->get_met (counter);
			spread_severity_tree (tnh, pcf, cubev, c, m, severity, sourceDir, counter);
		}
		else /* This may be a HWC */
		{
			double severity = LookForSlopeAt (prefix, countercode, Rstr, (breakpoints[u]-breakpoints[u-1])/2 + breakpoints[u-1]);
			cube::Metric *m = cubev->get_met (counter);
			spread_severity_tree (tnh, pcf, cubev, c, m, severity, sourceDir, counter);
		}
	}

#if 0
	{
		cube::Metric *m = cubev->get_met (counter); //
		cube::Thread *t = (cubev->get_thrdv())[0]; //
		cubev->set_sev (m, croot, t, value); //
	}
#endif

	return cubev->get_cnode_id (croot);
}

#if 0

void ca_callstackanalysis::do_analysis_presence_region (unsigned R, 
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

void ca_callstackanalysis::do_analysis_presence_region_cube_tree_r (
	TreeNodeHolder *tree, UIParaverTraceConfig *pcf, cube::Cube *cubev,
	cube::Cnode *root, string &sourceDir)
{
	cube::Metric *m = cubev->get_met ("no_Occurrences");
	cube::Thread *t = (cubev->get_thrdv())[0];

	for (unsigned u = 0; u < tree->children.size(); u++)
	{
		TreeNodeHolder *child = tree->children[u];

		int line;
		string file;
		common::lookForCallerLineInfo (pcf, child->CallerLine, file, line);

		string s;
		if (child->children.size() > 0)
			s = pcf->getEventValue (30000000, child->Caller);
		else
			s = pcf->getEventValue (30000000, child->Caller) + " - " + pcf->getEventValue (30000100,child->CallerLine);

		cube::Region* r = cubev->def_region (s, 0, 0, "", "", "");
		cube::Cnode* c = cubev->def_cnode (r, sourceDir+"/"+file, line, root);
		cubev->set_sev (m, c, t, child->Occurrences);

		do_analysis_presence_region_cube_tree_r (child, pcf, cubev, c, sourceDir);
	}
}
#endif

void ca_callstackanalysis::do_analysis_presence_region_cube_tree (TreeNodeHolder *tnh,
	UIParaverTraceConfig *pcf, cube::Cube *cubev, cube::Cnode *root,
	cube::Metric *m, string &sourceDir, string &metricName)
{
	cube::Thread *t = (cubev->get_thrdv())[0];
	for (unsigned u = 0; u < tnh->children.size(); u++)
	{
		TreeNodeHolder *child = tnh->children[u];
		cubev->set_sev (m, child->CubeNode, t, child->Occurrences);
		do_analysis_presence_region_cube_tree (child, pcf, cubev, NULL, m, sourceDir, metricName);
	}

	if (tnh->children.size() == 0)
	{
		int line;
		string file;
		common::lookForCallerLineInfo (pcf, tnh->CallerLine, file, line);

		ofstream source_metrics;
		source_metrics.open ((sourceDir+"/"+file+string(".metrics")).c_str(), ios::out|ios::app);
		if (source_metrics.is_open())
		{
			source_metrics << metricName << " " << line << " " << tnh->Occurrences << endl;
			source_metrics.close();
		}
	}

}

void ca_callstackanalysis::spread_severity_tree (TreeNodeHolder *tnh,
	UIParaverTraceConfig *pcf, cube::Cube *cubev, cube::Cnode *root,
	cube::Metric *m, double severity, string &sourceDir, string &metricName)
{
	for (unsigned u = 0; u < tnh->children.size(); u++)
	{
		TreeNodeHolder *child = tnh->children[u];
		spread_severity_tree (child, pcf, cubev, NULL, m, severity, sourceDir, metricName);
	}

	cube::Thread *t = (cubev->get_thrdv())[0];
	cubev->set_sev (m, tnh->CubeNode, t, tnh->children.size() == 0 ? severity : 0.0f);

	if (tnh->children.size() == 0)
	{
		int line;
		string file;
		common::lookForCallerLineInfo (pcf, tnh->CallerLine, file, line);

		ofstream source_metrics;
		source_metrics.open ((sourceDir+"/"+file+string(".metrics")).c_str(), ios::out|ios::app);
		if (source_metrics.is_open())
		{
			for (unsigned u = tnh->CallerLineASTBlock.first ; u <= tnh->CallerLineASTBlock.second; u++)
				source_metrics << metricName << " " << u << " " << severity << endl;
			source_metrics.close();
		}
	}
}

void ca_callstackanalysis::spread_constant_tree (TreeNodeHolder *tnh,
	UIParaverTraceConfig *pcf, double constant, string &sourceDir, string &Name)
{
	for (unsigned u = 0; u < tnh->children.size(); u++)
	{
		TreeNodeHolder *child = tnh->children[u];
		{
			int line;
			string file;
			common::lookForCallerLineInfo (pcf, child->CallerLine, file, line);

			ofstream source_metrics;
			source_metrics.open ((sourceDir+"/"+file+string(".metrics")).c_str(), ios::out|ios::app);
			if (source_metrics.is_open())
			{
				for (unsigned u = child->CallerLineASTBlock.first ; u <= child->CallerLineASTBlock.second; u++)
					source_metrics << Name << " " << u << " " << constant << endl;
				source_metrics.close();
			}
		}
		spread_constant_tree (child, pcf, constant, sourceDir, Name);
	}

	if (tnh->children.size() == 0)
	{
		int line;
		string file;
		common::lookForCallerLineInfo (pcf, tnh->CallerLine, file, line);

		ofstream source_metrics;
		source_metrics.open ((sourceDir+"/"+file+string(".metrics")).c_str(), ios::out|ios::app);
		if (source_metrics.is_open())
		{
			for (unsigned u = tnh->CallerLineASTBlock.first ; u <= tnh->CallerLineASTBlock.second; u++)
				source_metrics << Name << " " << u << " " << constant << endl;
			source_metrics.close();
		}
	}
}
