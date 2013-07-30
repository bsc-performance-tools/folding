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

	public:
	string regionName;
	vector<Instance*> Instances;
	vector<InstanceGroup*> InstanceGroups;

	public:
	InstanceContainer (string, InstanceSeparator *is);

	void add (Instance *i);
	unsigned numInstances (void);
	unsigned numSamples (void);
	void splitInGroups (void);
	unsigned numGroups (void)
	  { return ngroups; }

	void removePreviousDataFiles (ObjectSelection *os, string fileprefix);
	void dumpGroupData (ObjectSelection *os, string fileprefix);
	void gnuplot (ObjectSelection *os, string fileprefix, StatisticType_t type);
	void python (ObjectSelection *os, string prefix, set<string> counters);
};

#endif


