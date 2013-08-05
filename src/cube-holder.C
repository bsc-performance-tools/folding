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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/callstackanalysis.C $
 | 
 | @last_commit: $Date: 2013-05-24 16:08:28 +0200 (dv, 24 mai 2013) $
 | @version:     $Revision: 1764 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id: callstackanalysis.C 1764 2013-05-24 14:08:28Z harald $";

#include "common.H"

#include "cube-holder.H"
#include <iostream>
#include <fstream>
#include <sstream>

CubeHolder::CubeHolder (set<string> &counters)
{
	mach = c.def_mach ("Machine", "");
	node = c.def_node ("Node", mach);
	proc0 = c.def_proc ("Process 0", 0, node);
	thrd0 = c.def_thrd ("Thread 0", 0, proc0);

	int ndims = 1;
	vector<long> dimv;
	vector<bool> periodv;
	dimv.push_back (1);
	periodv.push_back (true);
	Cartesian* cart = c.def_cart (ndims, dimv, periodv);

	vector<long> coord0;
	coord0.push_back(0);
	c.def_coords(cart, (Sysres*) thrd0, coord0);

	c.def_met ("# Occurrences", NO_OCCURRENCES, "INTEGER",
	  "occ", "", "", "Number of occurrences of the callstack", NULL);

	c.def_met ("Duration", DURATION, "INTEGER",
	  "occ", "", "", "Duration of each region", NULL);

    for (set<string>::iterator it = counters.begin(); it != counters.end(); it++)
    {
        if (common::isMIPS(*it))
            c.def_met ("MIPS", "MIPS", "FLOAT", "occ", "", "", "", NULL);
        else
            c.def_met (*it+"/ms", *it+"pms", "FLOAT", "occ", "", "", "", NULL);
    }
}

void CubeHolder::generateCubeTree (string name, InstanceContainer &ic,
	UIParaverTraceConfig *pcf)
{
	/* Create a node for this subtree */
	Region *r = c.def_region (name, 0, 0, "", "", "");
	Cnode *cnode = c.def_cnode (r, "", 0, NULL);

	for (unsigned g = 0; g < ic.numGroups(); g++)
	{
		stringstream ss;
		ss << "Group " << g+1;

		/* Create a node for this subtree */
		Region *rtmp = c.def_region (ss.str(), 0, 0, "", "", "");
		Cnode *ctmp = c.def_cnode (rtmp, "", 0, cnode);

		/* Get the trees for each phase */
		vector<CallstackTree*> trees = ic.InstanceGroups[g]->getCallstackTrees();

		for (unsigned ph = 0; ph < trees.size(); ph++)
		{
			ss.str (string());
			ss << "Phase " << ph+1;

			/* Create a node for this subtree */
			Region *rtmp2 = c.def_region (ss.str(), 0, 0, "", "", "");
			Cnode *ctmp2 = c.def_cnode (rtmp2, "", 0, ctmp);

			/* Generate the remaining tree */
			if (trees[ph] != NULL)
			{
				CubeTree *ct = new CubeTree;
				CallstackTree *cst = trees[ph];
				ct->generate (c, ctmp2, cst, pcf);
			}
		}

	}
}

void CubeHolder::dump (string file)
{
	ofstream out (file.c_str());
	out << c;
	out.close();

#warning Create .launch generation
#if 0
			if (writecube)
			{
				ofstream output_cube_launch;
				output_cube_launch.open ((filePrefix+".launch").c_str(), ios::out|ios::app);
				if (!output_cube_launch.is_open())
				{
					cerr << "Error! Cannot create " << completefilePrefix+".launch! Dying..." << endl;
					exit (-1);
				}

				unsigned long long deltamS = ((*i)->Tend - (*i)->Tstart) / 1000000;
				unsigned long long counter_deltamS = (deltamS > 0) ? ((*i)->HWCvalues[CID]/deltamS) : 0;

				if (common::isMIPS(CounterID))
				{
					unsigned cnodeid = ca_callstackanalysis::do_analysis (filePrefix, "no_Occurrences", "no_Occurrences", 0,
						TranslateRegion(RegionName), RegionName, mbreakpoints[RegionName],
						vcallstacksamples, pcf, cube, sourceDirectory);

					output_cube_launch << "no_Occurrences" << endl
					  << "- cnode " << cnodeid << endl
					  << "See all detailed counters" << endl
					  << "gnuplot -persist %f." << RegionName << ".slopes.gnuplot" << endl;

					cnodeid = ca_callstackanalysis::do_analysis (filePrefix, "duration", "duration", (*i)->Tend - (*i)->Tstart,
						TranslateRegion(RegionName), RegionName, mbreakpoints[RegionName],
						vcallstacksamples, pcf, cube, sourceDirectory);

					output_cube_launch << "duration" << endl
					  << "- cnode " << cnodeid << endl
					  << "See all detailed counters" << endl
					  << "gnuplot -persist %f." << RegionName << ".slopes.gnuplot" << endl;
				}

				unsigned cnodeid;

				unsigned CounterIDcode = common::lookForCounter (CounterID, pcf);
				unsigned long long delta = (*i)->Tend - (*i)->Tstart;
				unsigned long long Tend = (*i)->Tend + 5*delta/100;
				unsigned long long Tstart = (5*delta/100 > (*i)->Tstart) ? 0 : (*i)->Tstart - 5*delta/100;

				if (common::isMIPS(CounterID))
					cnodeid = ca_callstackanalysis::do_analysis (filePrefix,
						CounterID, "MIPS", counter_deltamS /* info->mean_counter */,
						TranslateRegion(RegionName), RegionName, mbreakpoints[RegionName],
						vcallstacksamples, pcf, cube, sourceDirectory);
				else
					cnodeid = ca_callstackanalysis::do_analysis (filePrefix,
						CounterID, CounterID+"pms", counter_deltamS /* info->mean_counter */,
						TranslateRegion(RegionName), RegionName, mbreakpoints[RegionName],
						vcallstacksamples, pcf, cube, sourceDirectory);

				string nCounterID;
				if (common::isMIPS(CounterID))
					nCounterID = "MIPS";
				else
					nCounterID = CounterID+"pms";

				output_cube_launch << nCounterID << endl
				  << "- cnode " << cnodeid << endl
				  << "See detailed " << CounterID << endl
				  << "gnuplot -persist %f." << RegionName << "." << CounterID << ".gnuplot" << endl
				  << nCounterID << endl
				  << "- cnode " << cnodeid << endl
				  << "See " << CounterID << " in Paraver timeline" << endl;
				output_cube_launch << "folding-cube-call-paraver.sh " << TraceToFeed << " " << Tstart << " " << Tend << " " << FOLDED_BASE+CounterIDcode << " Folded_" << CounterID << " " << feedTraceFoldType_Value << " " << feedTraceFoldType_Value_Definition << endl;

				output_cube_launch.close();
			}
#endif

}

