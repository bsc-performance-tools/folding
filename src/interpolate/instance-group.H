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
# include <cube/Cube.h>
#endif

#include "model.H"
#include "data-object.H"
#include "callstack-processor.H"

using namespace std;
#if defined(HAVE_CUBE)
using namespace cube;
#endif

class InterpolationResults;
class CallstackTree;

class InstanceGroup
{
	private:
	const string regionName;
	const unsigned numGroup;
	const string groupName;

	bool preparedCallstacks;

	vector<Instance*> Instances;
	vector<Instance*> excludedInstances;

	map<string, InterpolationResults*> interpolated; /* map of InterpolationResults* indexed by counter */

	map<string, vector<Sample*> > used;
	map<string, vector<Sample*> > unused;
	set<Sample*> allSamples;

	vector<double> Breakpoints; /* nb (breakpoints) = nphases+1 */
	vector<CallstackTree *> trees; /* one per phase */
	vector< map<unsigned, CodeRefTripletAccounting*> > accountsPerLine;/* one per phase */

	vector< CallstackProcessor_Result* > callerstime_Results;
	vector< Sample * > callerstime_processSamples;

#if defined(HAVE_CUBE)
	Cnode *cubeTree;
#endif

	string gnuplot_model (const ObjectSelection *os, const string & prefix,
	  const Model *m, const string & TimeUnit);
	string gnuplot_addresses (const ObjectSelection *os, const string & prefix,
	  const string & TimeUnit, unsigned long long minaddress,
	  unsigned long long maxaddress); 

	public:
	InstanceGroup (string, unsigned, string);
	void setSamples (map<string, vector<Sample*> > & used,
	  map<string, vector<Sample*> > & unused);
	unsigned getNumGroup (void) const
	  { return numGroup; }
	string getGroupName (void) const
	  { return groupName; }
	string getRegionName (void) const
	  { return regionName; }
	map<string, vector<Sample*> > getSamples (void)
	  { return used; }
	map<string, vector<Sample*> > getUnusedSamples (void) 
	  { return unused; }
	set<Sample*> getAllSamples (void) const
	  { return allSamples; }
	vector<Instance*> getInstances(void) const
	  { return Instances; };
	void add (Instance *i);
	unsigned numInstances (void) const
	  { return Instances.size(); };
	unsigned numSamples (void) const;
	vector<Instance*> getExcludedInstances(void) const
	  { return excludedInstances; };
	void moveToExcluded (Instance *i);
	unsigned numExcludedInstances (void) const;
	unsigned numExcludedSamples (string counter) const;
	unsigned numExcludedSamples (void) const;

	unsigned long long mean (const string &counter) const; // Return the mean of counter within instances
	unsigned long long mean (void) const;  // Return the mean duration in time
	unsigned long long median (void) const;
	double stdev (void) const;
	unsigned long long MAD (void) const;

	void setInterpolated (map<string, InterpolationResults*> i)
	  { interpolated = i; }
	map<string, InterpolationResults*> getInterpolated (void) const
	  { return interpolated; }

	double getInterpolatedNTime (const string &counter, Sample *s) const;

	void setInterpolationBreakpoints (vector <double> breakpoints)
	  { Breakpoints = breakpoints; }
	vector<double> getInterpolationBreakpoints (void) const
	  { return Breakpoints; }

	void removePreviousData (ObjectSelection *os, const string & prefix);
	void dumpData (ObjectSelection *os, const string & prefix,
	  UIParaverTraceConfig *pcf);
	void dumpInterpolatedData (ObjectSelection *os, const string & prefix,
	  const vector<Model*> &models);
	void gnuplot (const ObjectSelection *os, const string & prefix,
	  const vector<Model*> &models, const string & TimeUnit,
	  vector<DataObject*> & vars, UIParaverTraceConfig *pcf);
	string python (void);

	void setCallstackTrees (vector<CallstackTree*> vtrees)
	  { trees = vtrees; }
	vector<CallstackTree*> getCallstackTrees (void) const
	  { return trees; }

#if defined(HAVE_CUBE)
	void setGeneralCubeTree (Cnode *cn)
	  { cubeTree = cn; }
	Cnode* getGeneralCubeTree (void) const
	  { return cubeTree; }
#endif

	void setAccountingPerLine (vector< map<unsigned, CodeRefTripletAccounting*> > axline)
	  { accountsPerLine = axline; }
	vector< map<unsigned, CodeRefTripletAccounting*> > getAccountingPerLine (void)
	  { return accountsPerLine; }
	
	bool hasAddresses (void) const;

	void prepareCallstacks (CallstackProcessor *processor);
	const vector<CallstackProcessor_Result*> getPreparedCallstacks (void) const
	  { return callerstime_Results; }
	vector<Sample*> getPreparedCallstacks_Process_Samples (void) const
	  { return callerstime_processSamples; }

	bool hasPreparedCallstacks (void) const
	  { return preparedCallstacks; }
};

#endif

