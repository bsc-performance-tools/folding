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
#include "pcf-common.H"

#include "cube-holder.H"
#include "interpolation-results.H"

#include <iostream>
#include <fstream>
#include <sstream>

CubeHolder::CubeHolder (UIParaverTraceConfig *pcf, const set<string> &ctrs)
	: counters(ctrs)
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

    for (const auto & ctr : counters)
    {
        if (common::isMIPS(ctr))
            c.def_met ("MIPS", "MIPS", "FLOAT", "occ", "", "", "", NULL);
        else
            c.def_met (ctr+"/ms", ctr+"pms", "FLOAT", "occ", "", "", "", NULL);
    }
}

void CubeHolder::generateCubeTree (InstanceContainer &ic,
	const string &sourceDir)
{
	string name = ic.getRegionName();

	/* Create a node for this subtree */
	Region *r = c.def_region (name, name, "", "", 0, 0, "", "", "");
	Cnode *cnode = c.def_cnode (r, "", 0, NULL);

	for (unsigned g = 0; g < ic.numGroups(); g++)
	{
		stringstream ss;
		ss << "Group " << g+1;

		/* Create a node for this subtree */
		Region *rtmp = c.def_region (ss.str(), ss.str(), "", "", 0, 0, "", "", "");
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
			Region *rtmp2 = c.def_region (ss.str(), ss.str(), "", "", 0, 0, "", "", "");
			Cnode *ctmp2 = c.def_cnode (rtmp2, "", 0, ctmp);

			/* Generate the remaining tree */
			if (trees[ph] != NULL)
			{
				CubeTree *ct = new CubeTree;
				CallstackTree *cst = trees[ph];
				ct->generate (c, ctmp2, cst, pcf, sourceDir);

				/* Insert into hash for future use in setCubeSeverities */
				rootNodes[std::make_pair (ig, ph)] = std::make_pair(ctmp2,ct);
			}
		}
	}
}

void CubeHolder::setCubeSeverities (void)
{
	for (const auto & rn : rootNodes)
	{
		/* Apply to top level first, groups & phases */
		pair<InstanceGroup*,unsigned> instance_phase = rn.first;
		pair<Cnode*,CubeTree*> cubenode_tree = rn.second;
		Cnode *node = cubenode_tree.first;
		setSeverities (node, instance_phase.first, instance_phase.second);

		/* Apply to each level within a subtree of every group and phase */
		CubeTree *ct = cubenode_tree.second;
		ct->setSeverities (c);
	}
}

void CubeHolder::setSeverities (Cnode *node, InstanceGroup *ig, unsigned phase)
{
	vector<double> bpts = ig->getInterpolationBreakpoints();
	double portion = bpts[phase+1] - bpts[phase];
	double inbetween = (bpts[phase+1] + bpts[phase]) / 2.0f;

	double severity = (ig->mean()) * portion;
	Metric *metric = c.get_met (DURATION);
	cube::Thread *t = (c.get_thrdv())[0];
	c.set_sev (metric, node, t, severity);

	map<string, InterpolationResults*> iresults = ig->getInterpolated();
	for (const auto & ctr : counters)
		if (iresults.count (ctr) > 0)
		{
			string nCounterID;
			if (common::isMIPS(ctr))
				nCounterID = "MIPS";
			else
				nCounterID = ctr+"pms";

			severity = (iresults[ctr])->getSlopeAt (inbetween);
			metric = c.get_met (nCounterID);
			cube::Thread *t = (c.get_thrdv())[0];
			c.set_sev (metric, node, t, severity);
		}
}

void CubeHolder::dump (const string & file)
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

void CubeHolder::eraseLaunch (const string & file)
{
	string f = file.substr (0, file.rfind (".folded")) + ".folded.launch";

	if (common::existsFile(f))
		if (unlink(f.c_str()))
			cerr << "Warning! Could not remove " << f << endl;
}

void CubeHolder::EmitMetricFileLine (const string &dir, const string &file,
	const string &region, unsigned phase, const string & metric, unsigned line,
	unsigned val)
{
	ofstream f((dir+"/"+file+"."+region+".metrics").c_str(), std::ofstream::app);
	if (f.is_open())
		f << phase << " " << metric << " " << line << " " << val << endl;
	f.close();
}

void CubeHolder::EmitMetricFileLine (const string &dir, const string &file,
	const string &region, unsigned phase, const string & metric, unsigned line,
	double val)
{
	ofstream f((dir+"/"+file+"."+region+".metrics").c_str(), std::ofstream::app);
	if (f.is_open())
		f << phase << " " << metric << " " << line << " " << val << endl;
	f.close();
}

void CubeHolder::dumpFileMetrics_Lines_ASTs (const string & dir,
	InstanceGroup *ig)
{
	string region = ig->getRegionName();
	map<string, InterpolationResults*> iresults = ig->getInterpolated();
	vector< map< unsigned, CodeRefTripletAccounting* > > aXline = ig->getAccountingPerLine();
	vector<double> bpts = ig->getInterpolationBreakpoints();

	string MIPS_ctr;
	for (const auto & ctr : counters)
		if (common::isMIPS(ctr))
			MIPS_ctr = ctr;

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
			pcfcommon::lookForCallerFullInfo (pcf, crt.getCaller(), crt.getCallerLine(),
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

						for (const auto & ctr : counters)
						{
							string nCounterID;
							if (common::isMIPS(ctr))
								nCounterID = "MIPS";
							else
								nCounterID = ctr+"pms";

							double hwcvalue = (iresults[ctr])->getSlopeAt (inbetween);
							EmitMetricFileLine (dir, filename, region, phase+1,
							  nCounterID, l, hwcvalue);
							if (!common::isMIPS(ctr))
							{
								double hwcvalue_ins = (iresults[MIPS_ctr])->getSlopeAt (inbetween);
								if (hwcvalue > 0)
									EmitMetricFileLine (dir, filename, region, phase+1,
									  ctr+"_per_ins", l, hwcvalue/hwcvalue_ins);
								else
									EmitMetricFileLine (dir, filename, region, phase+1,
									  ctr+"_per_ins", l, 0.0);
							}
						}
					}
					processedASTs.insert (crt.getCallerLineAST());
				}
			}
		}
	}
}

void CubeHolder::dumpFileMetrics (const string & dir, InstanceContainer &ic)
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

		dumpFileMetrics_Lines_ASTs (dir, maxig);
	}
}

