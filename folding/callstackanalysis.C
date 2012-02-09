/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  MPItrace                                 *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *                                                             ___           *
 *   +---------+     http:// www.cepba.upc.edu/tools_i.htm    /  __          *
 *   |    o//o |     http:// www.bsc.es                      /  /  _____     *
 *   |   o//o  |                                            /  /  /     \    *
 *   |  o//o   |     E-mail: cepbatools@cepba.upc.edu      (  (  ( B S C )   *
 *   | o//o    |     Phone:          +34-93-401 71 78       \  \  \_____/    *
 *   +---------+     Fax:            +34-93-401 25 77        \  \__          *
 *    C E P B A                                               \___           *
 *                                                                           *
 * This software is subject to the terms of the CEPBA/BSC license agreement. *
 *      You must accept the terms of this license to use this software.      *
 *                                 ---------                                 *
 *                European Center for Parallelism of Barcelona               *
 *                      Barcelona Supercomputing Center                      *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/folding/interpolate.C $
 | 
 | @last_commit: $Date: 2012-01-27 16:02:20 +0100 (dv, 27 gen 2012) $
 | @version:     $Revision: 948 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id: interpolate.C 948 2012-01-27 15:02:20Z harald $";

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
		  samples, pcf, cube, c);

		if (pcf != NULL)
			do_analysis_presence_region_cube (R, Rstr, u, breakpoints[u-1],
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
			if (100*found_paths[u].occurrences >= 5*TOTALoccurrences) /* show only above 5% */
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

void ca_callstackanalysis::do_analysis_presence_region_cube (unsigned R, string Rstr,
	unsigned phase,	double from, double to, vector<ca_callstacksample> &samples,
	UIParaverTraceConfig *pcf, cube::Cube *cube, cube::Cnode *base)
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
			cube::Metric *m = cube->get_met ("no_Occurrences");
			cube::Thread *t = (cube->get_thrdv())[0];
			cube::Cnode *previous = base;
			unsigned NN = found_paths[u].caller.size();
			for (unsigned uu = 0; uu < NN; uu++)
			{
				string s = pcf->getEventValue (30000000, found_paths[u].caller[uu]) + " - " + pcf->getEventValue (30000100,found_paths[u].callerline[uu]);
				cube::Region* r = cube->def_region (s, 0, 0, "", "", "");
				cube::Cnode* c = cube->def_cnode (r, "", 0, previous);
				cube->set_sev (m, c, t, found_paths[u].occurrences);
					previous = c;
				}
			}
		}
	}
}

