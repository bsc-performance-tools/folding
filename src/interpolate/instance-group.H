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

/* This class contains information regarding the instances read from the
   extract tool but grouped according to an InstanceSeparator. The InstanceGroup
   is the main collection of instances used through the interpolate  from the
   folding tool as each analysis is typically done per InstanceGroup rather than
   per Instance. Note that it contains both used and excluded instances. The
   separation between the two depends on the user requests */

class InstanceGroup
{
	private:
	/* Name of the instance */
	const string regionName;
	/* Assigned group number */
	const unsigned numGroup;
	/* Assigned group name */
	const string groupName;

	/* Have we processed the callstack information? */
	bool preparedCallstacks;
	/* Have we emitted the callstack information into the CSV file? */
	bool hasEmittedCallstackSamples;
	/* Have we emitted the address information into the CSV file? */
	bool hasEmittedAddressSamples;

	/* The container of the used instances */
	vector<Instance*> Instances;

	/* The container of the excluded instances. Excluded as the user requests
	   given the outliers / sigma values */
	vector<Instance*> excludedInstances;

	/* Map (key = counter, value = interpolation results) provides the result
	   of the interpolation per counter within this instance group using the
	   not excluded instances */
	map<string, InterpolationResults*> interpolated;

	/* Selection of used samples according to the user parametrization for
	   each counter (key) */
	map<string, vector<Sample*> > used;

	/* Selection of unused samples according to the user parametrization for
	   each counter (key) */
	map<string, vector<Sample*> > unused;

	/* Set of all samples for the instances */
	set<Sample*> allSamples;

	/* If using R breakpoints, this will contain the # of breakpoints found in
	   the interpolation */
	vector<double> Breakpoints; /* nb (breakpoints) = nphases+1 */

	/* Constructs a callstack-tree based on the callstack information. This is
	   valuable to generate the CUBE output */
	vector<CallstackTree *> trees; /* one per phase */

	/* Attributes to each phase derived from the breakpoints the number of visits */
	vector< map<unsigned, CodeRefTripletAccounting*> > accountsPerLine;/* one per phase */

	/* Results of the callstack processor, using the bio-inspired approach */
	vector< CallstackProcessor_Result* > callerstime_Results;

	/* Here we store the processed samples using the bio-inspired approach to
	   later dump the information into the Paraver trace-file */
	vector< Sample * > callerstime_processSamples;

#if defined(HAVE_CUBE)
	/* Cube hook */
	Cnode *cubeTree;
#endif

	/* Generate the gnuplot using a performance architecture model */
	string gnuplot_model (const ObjectSelection *os, const string & prefix,
	  const Model *m, const string & TimeUnit);

	/* Generate the gnuplot for the access to the address space */
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

	void setHasEmittedCallstackSamples (bool b)
	  { hasEmittedCallstackSamples = b; }
	bool getHasEmittedCallstackSamples (void) const
	  { return hasEmittedCallstackSamples; }

	void setHasEmittedAddressSamples (bool b)
	  { hasEmittedAddressSamples = b; }
	bool getHasEmittedAddressSamples (void) const
	  { return hasEmittedAddressSamples; }

#if defined(DAMIEN_EXPERIMENTS)
	void DumpReverseCorrectedCallersInInstance (const Instance *in, bool first,
	  const string &fprefix, UIParaverTraceConfig *pcf) const;
#endif
};

#endif

