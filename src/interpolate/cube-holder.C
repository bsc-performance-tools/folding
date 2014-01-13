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

#include "cube-holder.H"
#include "interpolation-results.H"

#include <iostream>
#include <fstream>
#include <sstream>

CubeHolder::CubeHolder (UIParaverTraceConfig *pcf, set<string> &counters)
{
	this->pcf = pcf;

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

void CubeHolder::generateCubeTree (InstanceContainer &ic, UIParaverTraceConfig *pcf,
	string &sourceDir, set<string> counters)
{
	string name = ic.getRegionName();

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

		InstanceGroup *ig = ic.getInstanceGroup(g);

		ig->setGeneralCubeTree (ctmp);

		/* Get the trees for each phase */
		vector<CallstackTree*> trees = ig->getCallstackTrees();

		vector<double> bpts = ig->getInterpolationBreakpoints();

		for (unsigned ph = 0; ph < trees.size(); ph++)
		{
			ss.str (string());
			ss << "Phase " << ph+1 << " [" << common::convertDouble (bpts[ph], 2)
			  << "-" << common::convertDouble (bpts[ph+1], 2) << "]";


			/* Create a node for this subtree */
			Region *rtmp2 = c.def_region (ss.str(), 0, 0, "", "", "");
			Cnode *ctmp2 = c.def_cnode (rtmp2, "", 0, ctmp);

			/* Generate the remaining tree */
			if (trees[ph] != NULL)
			{
				CubeTree *ct = new CubeTree;
				CallstackTree *cst = trees[ph];
				ct->generate (c, ctmp2, cst, pcf, sourceDir);

				setSeverities (ctmp2, ig, ph, counters); 
			}
		}
	}
}


void CubeHolder::setSeverities (Cnode *node, InstanceGroup *ig, unsigned phase,
	set<string> counters)
{
	vector<double> bpts = ig->getInterpolationBreakpoints();
	double portion = bpts[phase+1] - bpts[phase];
	double inbetween = (bpts[phase+1] + bpts[phase]) / 2.0f;

	double severity = (ig->mean()) * portion;
	Metric *metric = c.get_met (DURATION);
	cube::Thread *t = (c.get_thrdv())[0];
	c.set_sev (metric, node, t, severity);

	map<string, InterpolationResults*> iresults = ig->getInterpolated();
	set<string>::iterator ctr;
	for (ctr = counters.begin(); ctr != counters.end(); ctr++)
		if (iresults.count (*ctr) > 0)
		{
			string nCounterID;
			if (common::isMIPS(*ctr))
				nCounterID = "MIPS";
			else
				nCounterID = (*ctr)+"pms";

			severity = (iresults[*ctr])->getSlopeAt (inbetween);
			metric = c.get_met (nCounterID);
			cube::Thread *t = (c.get_thrdv())[0];
			c.set_sev (metric, node, t, severity);
		}
}

void CubeHolder::dump (string file)
{
	ofstream out (file.c_str());
	if (out.is_open())
	{
		out << c;
		out.close();
	}
	else
		cerr << "Error! Cannot create " << file << endl;
}

void CubeHolder::eraseLaunch (string file)
{
	string f = file.substr (0, file.rfind (".folded")) + ".folded.launch";

	if (common::existsFile(f))
		if (unlink(f.c_str()))
			cerr << "Warning! Could not remove " << f << endl;
}

void CubeHolder::dumpLaunch (InstanceContainer &ic, ObjectSelection *os,
	set<string> counters, string file)
{
	string prefix = file.substr (0, file.rfind (".folded"));

	ofstream launch ((prefix + ".folded.launch").c_str(), std::ofstream::app);
	if (launch.is_open())
	{
		string RegionName = ic.getRegionName();

		for (unsigned g = 0; g < ic.numGroups(); g++)
		{
			InstanceGroup *ig = ic.getInstanceGroup(g);

			/* Get the group name */
			string groupName = ig->getGroupName();

			/* Get the general tree for this group */
			Cnode *tree = ig->getGeneralCubeTree();

			string gname = prefix + "." + common::removeUnwantedChars(RegionName) + "." + 
			  os->toString (false, "any") + "." + common::removeSpaces (groupName);

			unsigned cnodeid = c.get_cnode_id (tree);

			launch
			  << DURATION << endl
			  << "- cnode " << cnodeid << endl
			  << "See all detailed counters" << endl
			  << "gnuplot -persist " << gname << ".slopes.gnuplot" << endl;
			launch
			  << NO_OCCURRENCES << endl
			  << "- cnode " << cnodeid << endl
			  << "See all detailed counters" << endl
			  << "gnuplot -persist " << gname << ".slopes.gnuplot" << endl;

			set<string>::iterator ctr;
			for (ctr = counters.begin(); ctr != counters.end(); ctr++)
			{
				launch
				  << DURATION << endl
				  << "- cnode " << cnodeid << endl
				  << "See detailed " << *ctr << endl
				  << "gnuplot -persist " << gname  << "." << *ctr
				  << ".gnuplot" << endl;
				launch
				  << NO_OCCURRENCES << endl
				  << "- cnode " << cnodeid << endl
				  << "See detailed " << *ctr << endl
				  << "gnuplot -persist " << gname  << "." << *ctr
				  << ".gnuplot" << endl;

				string nCounterID;
				if (common::isMIPS(*ctr))
					nCounterID = "MIPS";
				else
					nCounterID = (*ctr)+"pms";

				launch
				  << nCounterID << endl
				  << "- cnode " << cnodeid << endl
				  << "See detailed " << *ctr << endl
				  << "gnuplot -persist " << gname  << "." << *ctr
				  << ".gnuplot" << endl;
			}
		}
	}
	else
		cerr << "Error! Cannot create " << file << endl;

	launch.close();
}

void CubeHolder::EmitMetricFileLine (string &dir, string &file, string &region, 
	unsigned phase, string metric, unsigned line, unsigned val)
{
	ofstream f((dir+"/"+file+"."+region+".metrics").c_str(), std::ofstream::app);
	if (f.is_open())
		f << phase << " " << metric << " " << line << " " << val << endl;
	f.close();
}

void CubeHolder::EmitMetricFileLine (string &dir, string &file, string &region, 
	unsigned phase, string metric, unsigned line, double val)
{
	ofstream f((dir+"/"+file+"."+region+".metrics").c_str(), std::ofstream::app);
	if (f.is_open())
		f << phase << " " << metric << " " << line << " " << val << endl;
	f.close();
}

void CubeHolder::dumpFileMetrics_Lines_ASTs (string dir, InstanceGroup *ig,
	set<string> counters)
{
	string region = ig->getRegion();
	map<string, InterpolationResults*> iresults = ig->getInterpolated();
	vector< map< unsigned, CodeRefTripletAccounting* > > aXline = ig->getAccountingPerLine();
	vector<double> bpts = ig->getInterpolationBreakpoints();

	map<unsigned, unsigned> totalCounts;
	map<unsigned, map<unsigned, unsigned> > totalCountsPerPhase;
	for (unsigned phase = 0; phase < aXline.size(); phase++)
	{
		map<unsigned, CodeRefTripletAccounting*> accPerLine = aXline[phase];
		map<unsigned, CodeRefTripletAccounting*>::iterator line;

		/* Calculate the threshold per AST */
		for (line = accPerLine.begin(); line != accPerLine.end(); line++)
		{
			CodeRefTriplet crt = (*line).second->getCodeTriplet();

			if (totalCounts.count (crt.getCallerLineAST()) == 0)
				totalCounts[crt.getCallerLineAST()] = ((*line).second)->getCount();
			else
				totalCounts[crt.getCallerLineAST()] += ((*line).second)->getCount();

			if (totalCountsPerPhase.count(crt.getCallerLineAST()) == 0)
			{
				map<unsigned, unsigned> CountPerPhase;
				CountPerPhase[phase] = ((*line).second)->getCount();
				totalCountsPerPhase[crt.getCallerLineAST()] = CountPerPhase;
			}
			else
			{
				map<unsigned, unsigned> CountPerPhase = totalCountsPerPhase[crt.getCallerLineAST()];
				CountPerPhase[phase] += ((*line).second)->getCount();
				totalCountsPerPhase[crt.getCallerLineAST()] = CountPerPhase;
			}
		}
	}

	for (unsigned phase = 0; phase < aXline.size(); phase++)
	{
		double portion = bpts[phase+1] - bpts[phase];
		double duration = (ig->mean()) * portion;
		double inbetween = (bpts[phase+1] + bpts[phase]) / 2.0f;

		map<unsigned, CodeRefTripletAccounting*> accPerLine = aXline[phase];
		map<unsigned, CodeRefTripletAccounting*>::iterator line;

		/* Annotate here the ASTs where we have dumped info in order to avoid
		   duplications */
		set<unsigned> processedASTs;

		for (line = accPerLine.begin(); line != accPerLine.end(); line++)
		{
			CodeRefTriplet crt = (*line).second->getCodeTriplet();
			string filename;
			string routine;
			unsigned fline, bline, eline;
			common::lookForCallerFullInfo (pcf, crt.getCaller(), crt.getCallerLine(),
			  crt.getCallerLineAST(), routine, filename, fline, bline, eline);

			map<unsigned, unsigned> CountsPerPhase = totalCountsPerPhase[crt.getCallerLineAST()];
			if (CountsPerPhase[phase] >= (totalCounts[crt.getCallerLineAST()] * 5) / 100)
			{
				EmitMetricFileLine (dir, filename, region, phase+1,
				  "#Occurrences", fline, ((*line).second)->getCount());

				if (processedASTs.count(crt.getCallerLineAST()) == 0)
				{
					for (unsigned l = bline; l <= eline; l++)
					{
						/* Emit phase, duration & hwcounters */
						EmitMetricFileLine (dir, filename, region, phase+1,
						  "Duration(ms)", l, duration / 1000000.f);

						set<string>::iterator ctr;
						for (ctr = counters.begin(); ctr != counters.end(); ctr++)
						{
							string nCounterID;
							if (common::isMIPS(*ctr))
								nCounterID = "MIPS";
							else
								nCounterID = (*ctr)+"pms";

							double hwcvalue = (iresults[*ctr])->getSlopeAt (inbetween);
							EmitMetricFileLine (dir, filename, region, phase+1,
							  nCounterID, l, hwcvalue);
						}
					}
					processedASTs.insert (crt.getCallerLineAST());
				}
			}
		}
	}
}

void CubeHolder::dumpFileMetrics (string dir, InstanceContainer &ic, set<string> counters)
{
	if (common::existsDir (dir))
	{
		unsigned maxgroup = 0;
		InstanceGroup *maxig = ic.getInstanceGroup(maxgroup);
		for (unsigned g = 1; g < ic.numGroups(); g++)
		{
			InstanceGroup *ig = ic.getInstanceGroup(g);
			if (maxig->numInstances() < ig->numInstances())
			{
				maxgroup = g;
				maxig = ic.getInstanceGroup(maxgroup);
			}
		}

		dumpFileMetrics_Lines_ASTs (dir, maxig, counters);
	}
}

