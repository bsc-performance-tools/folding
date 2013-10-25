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

#include "cube-tree.H"
#include "cube-holder.H"

#include <iostream>
#include <sstream>

void CubeTree::generate (Cube &c, Cnode *parent, CallstackTree *ctree, 
	UIParaverTraceConfig *pcf, string &sourceDir, unsigned depth)
{
	CodeRefTriplet crt = ctree->getCodeRefTriplet();

	string routine, file;
	int bline, eline;

	/* If this is the top of the tree and the node is a fake main,
	   rename the routine into "main*" */
	if (depth == 0 && crt.getCaller() == 0)
	{
		routine = "main*";
		file = "";
		bline = eline = 0;
	}
	else
		common::lookForCallerASTInfo (pcf, crt.getCaller(), crt.getCallerLineAST(),
			routine, file, bline, eline);

	/* Create a node for this routine */
	cube_region = c.def_region (routine, 0, 0, "", "", file);
	cube_node = c.def_cnode (cube_region, sourceDir + "/" + file, 0, parent);
	tree_node = ctree;

	/* Store # occurrences */
	Metric *m = c.get_met (NO_OCCURRENCES);
	Thread *t = (c.get_thrdv())[0];
	c.set_sev (m, cube_node, t, ctree->getOccurrences());

	vector<CallstackTree*> vctree = ctree->getChildren();
	if (vctree.size() > 0)
	{
		for (unsigned u = 0; u < vctree.size(); u++)
		{
			CubeTree *tmp = new CubeTree;
			tmp->generate (c, cube_node, vctree[u], pcf, sourceDir, depth+1);
			children.push_back (tmp);
		}
	}
}

