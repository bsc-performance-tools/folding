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

#ifndef CUBEHOLDER_H_INCLUDED
#define CUBEHOLDER_H_INCLUDED

#include <cube3/Cube.h>

#include "cube-tree.H"
#include "instance-container.H"
#include "UIParaverTraceConfig.h"

#include <set>

using namespace cube;
using namespace std;

#define NO_OCCURRENCES "no_Occurrences"
#define DURATION       "Duration"

class CubeHolder
{
	private:
	UIParaverTraceConfig *pcf;
	Cube c;

	Machine *mach;
	Node *node;
	Process *proc0;
	Thread *thrd0;

	void setSeverities (Cnode *node, InstanceGroup *ig, unsigned phase,
	  set<string> counters);

	void dumpFileMetrics_Lines (string dir, InstanceGroup *ig,
	  set<string> counters);
	void dumpFileMetrics_ASTs (string dir, InstanceGroup *ig,
	  set<string> counters);

	void EmitMetricFileLine (string dir, string metric, string file, 
	  unsigned line, unsigned val);
	void EmitMetricFileLine (string dir, string metric, string file,
	  unsigned line, double val);

	public:
	CubeHolder (UIParaverTraceConfig *pcf, set<string> &counters);
	void generateCubeTree (InstanceContainer &ic, UIParaverTraceConfig *pcf,
	  string &sourceDir, set<string> counters);
	void eraseLaunch (string file);
	void dumpLaunch (InstanceContainer &ic, ObjectSelection *os, 
	  set<string> counters, string file);
	void dump (string file);

	void dumpFileMetrics (string dir, InstanceContainer &ic,
	  set<string> counters);
};

#endif /* CUBEHOLDER_H_INCLUDED */
