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

#ifndef INSTANCECONTAINER_H_INCLUDED
#define INSTANCECONTAINER_H_INCLUDED

#include "instance-group.H"
#include "interpolate.H"
#include "instance-separator.H"
#include <vector>
#include <string>
#include <map>

using namespace std;

class InstanceContainer
{
	private:
	unsigned ngroups;
	InstanceSeparator *is;
	string regionName;
	vector<Instance*> Instances;
	vector<InstanceGroup*> InstanceGroups;

	public:
	InstanceContainer (string, InstanceSeparator *is);

	void add (Instance *i)
	  { Instances.push_back(i); }
	const vector<Instance *> getInstances (void) const
	  { return Instances; }
	unsigned numInstances (void) const
	  { return Instances.size(); }
	unsigned numSamples (void);
	void splitInGroups (void);
	unsigned numGroups (void) const
	  { return ngroups; }
	string getRegionName (void) const
	  { return regionName; }
	InstanceGroup* getInstanceGroup (unsigned g);
	InstanceSeparator* getInstanceSeparator(void) const
	  { return is; }

	void removePreviousDataFiles (const ObjectSelection *os,
	  const string & fileprefix);
	void dumpGroupData (const ObjectSelection *os,
	  const string & fileprefix, const string & TimeUnit);
	void gnuplot (const ObjectSelection *os, const string & fileprefix,
	  StatisticType_t type);
	void python (const ObjectSelection *os, const string & prefix, 
	  const set<string> & counters);
};

#endif


