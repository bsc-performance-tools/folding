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

#ifndef INSTANCEGROUP_H_INCLUDED
#define INSTANCEGROUP_H_INCLUDED

#include "instance.H"
#include "object-selection.H"
#include "codereftriplet-accounting.H"
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#if defined(HAVE_CUBE)
# include <cube3/Cube.h>
#endif

using namespace std;
#if defined(HAVE_CUBE)
using namespace cube;
#endif

class InterpolationResults;
class CallstackTree;
class CubeTree;

class InstanceGroup
{
	private:
	vector<Instance*> Instances;
	vector<Instance*> excludedInstances;

	map<string, InterpolationResults*> interpolated; /* counter to InterpolationResults* */

	map<string, vector<Sample*> > used;
	map<string, vector<Sample*> > unused;

	vector<double> Breakpoints; /* nb (breakpoints) = nphases+1 */
	vector<CallstackTree *> trees; /* one per phase */
	vector< map<unsigned, CodeRefTripletAccounting*> > accountsPerLine;/* one per phase */

#if defined(HAVE_CUBE)
	Cnode *cubeTree;
#endif

	string regionName;
	string groupName;
	unsigned numGroup;

	void gnuplot_single (ObjectSelection *os, string prefix, string counter,
	  InterpolationResults *ir);
	void gnuplot_slopes (ObjectSelection *os, string prefix);

	public:
	InstanceGroup (string, unsigned, string);

	void setSamples (map<string, vector<Sample*> > used, map<string, vector<Sample*> > unused)
		{ this->used = used; this->unused = unused; }

	unsigned getNumGroup (void)
	  { return numGroup; }

	string getGroupName (void)
	  { return groupName; }

	string getRegion (void)
	  { return regionName; }

	map<string, vector<Sample*> > getSamples (void)
		{ return this->used; }

	vector<Instance*> getInstances(void)
	  { return Instances; };
	void add (Instance *i);
	unsigned numInstances (void);
	unsigned numSamples (void);

	vector<Instance*> getExcludedInstances(void)
	  { return excludedInstances; };
	void moveToExcluded (Instance *i);
	unsigned numExcludedInstances (void);
	unsigned numExcludedSamples (string counter);
	unsigned numExcludedSamples (void);

	unsigned long long mean (void);
	unsigned long long median (void);
	double stdev (void);
	unsigned long long MAD (void);

	void setInterpolated (map<string, InterpolationResults*> i)
	  { interpolated = i; }
	map<string, InterpolationResults*> getInterpolated (void)
	  { return interpolated; }

	void removePreviousData (ObjectSelection *os, string prefix);
	void dumpData (ObjectSelection *os, string prefix);
	void dumpInterpolatedData (ObjectSelection *os, string prefix);
	void gnuplot (ObjectSelection *os, string prefix);
	string python (void);

	void setInterpolationBreakpoints (vector <double> breakpoints)
	  { Breakpoints = breakpoints; }
	vector<double> getInterpolationBreakpoints (void)
	  { return Breakpoints; }

	void setCallstackTrees (vector<CallstackTree*> vtrees)
	  { trees = vtrees; }
	vector<CallstackTree*> getCallstackTrees (void)
	  { return trees; }

#if defined(HAVE_CUBE)
	void setGeneralCubeTree (Cnode *cn)
	  { cubeTree = cn; }
	Cnode* getGeneralCubeTree (void)
	  { return cubeTree; }
#endif

	void setAccountingPerLine (vector< map<unsigned, CodeRefTripletAccounting*> > axline)
	  { accountsPerLine = axline; }
	vector< map<unsigned, CodeRefTripletAccounting*> > getAccountingPerLine (void)
	  { return accountsPerLine; }
};

#endif

