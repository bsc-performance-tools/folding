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
#ifndef INSTANCECONTAINER_H_INCLUDED
#define INSTANCECONTAINER_H_INCLUDED

#include "instance-group.H"
#include "interpolate.H"
#include "instance-separator.H"
#include <vector>
#include <string>
#include <map>

using namespace std;

/* This class represents a container for all the instances. */

class InstanceContainer
{
	private:
	/* Number of group detected */
	unsigned ngroups;

	/* Which instance separator to be used */
	InstanceSeparator *is;

	/* Name of the region */
	string regionName;

	/* Container of all instances */
	vector<Instance*> Instances;

	/* Container of instances grouped, that is each position contains a 
	   instance group according to the separator 'is' */
	vector<InstanceGroup*> InstanceGroups;

	public:
	InstanceContainer (string, InstanceSeparator *is);

	/* Add a new instance */
	void add (Instance *i)
	  { Instances.push_back(i); }

	/* Get all instances */
	const vector<Instance *> getInstances (void) const
	  { return Instances; }

	/* Get # of instances */
	unsigned numInstances (void) const
	  { return Instances.size(); }

	/* Get # of samples within instances */
	unsigned numSamples (void);

	/* Split instances into groups */
	void splitInGroups (void);

	/* Return the # of groups */
	unsigned numGroups (void) const
	  { return ngroups; }

	/* Get the name of the instance */
	string getRegionName (void) const
	  { return regionName; }

	/* Get a particular instance group (g) */
	InstanceGroup* getInstanceGroup (unsigned g);

	/* Get the intance separator */
	InstanceSeparator* getInstanceSeparator(void) const
	  { return is; }

	/* Remove previously existing files. Do some cleanup. */
	void removePreviousDataFiles (const ObjectSelection *os,
	  const string & fileprefix);

	/* Dumps instance information into CSV files for gnuplot plotting */
	void dumpGroupData (const ObjectSelection *os,
	  const string & fileprefix, const string & TimeUnit);

	/* Generate gnuplot scripts */
	void gnuplot (const ObjectSelection *os, const string & fileprefix,
	  StatisticType_t type);

	/* Generate python script info */
	void python (const ObjectSelection *os, const string & prefix, 
	  const set<string> & counters);
};

#endif


