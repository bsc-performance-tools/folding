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
#ifndef CUBEHOLDER_H_INCLUDED
#define CUBEHOLDER_H_INCLUDED

#include <cube/Cube.h>

#include "cube-tree.H"
#include "instance-container.H"
#include "UIParaverTraceConfig.h"

#include <set>
#include <map>

using namespace cube;
using namespace std;

#define NO_OCCURRENCES "no_Occurrences"
#define DURATION       "Duration"

class CubeHolder
{
	private:
	const set<string> counters;

	UIParaverTraceConfig *pcf;
	Cube c;

	Machine *mach;
	Node *node;
	Process *proc0;
	Thread *thrd0;

	map< pair<InstanceGroup*,unsigned>, pair<Cnode*,CubeTree*> > rootNodes;

	void setSeverities (Cnode *node, InstanceGroup *ig, unsigned phase);

	void dumpFileMetrics_Lines_ASTs (const string & dir, InstanceGroup *ig);

	void EmitMetricFileLine (const string &dir, const string &file,
	  const string &region, unsigned phase, const string & metric,
	  unsigned line, unsigned val);
	void EmitMetricFileLine (const string &dir, const string &file,
	  const string &region, unsigned phase, const string & metric,
	  unsigned line, double val);

	public:
	CubeHolder (UIParaverTraceConfig *pcf, const set<string> &counters);
	void generateCubeTree (InstanceContainer &ic, const string &sourceDir);
	void setCubeSeverities (void);
	void eraseLaunch (const string & file);
	void dump (const string & file);
	void dumpFileMetrics (const string & dir, InstanceContainer &ic);
};

#endif /* CUBEHOLDER_H_INCLUDED */
