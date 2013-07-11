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

static char __attribute__ ((unused)) rcsid[] = "$Id: callstackanalysis.C 1764 2013-05-24 14:08:28Z harald $";

#include "common.H"

#include "cube-tree.H"
#include "cube-holder.H"

#include <iostream>
#include <sstream>

#if 0

void CubeTree::generateLeaf (Cube &c, Cnode *parent, CodeRefTriplet crt,
	unsigned occurrences, UIParaverTraceConfig *pcf)
{
	string routine, file;
	int bline, eline;

	common::lookForFullCallerInfo (pcf, crt.Caller, crt.CallerLineAST, routine,
	  file, bline, eline);

	stringstream blines, elines;
	blines << bline;
	elines << eline;

	string s = routine + " (" + file;
	if (bline != eline)
		s += " [" + blines.str() + "-" + elines.str() + "])";
	else
		s += " [" + blines.str() + "])";

	/* Create a node for this subtree */
	cube_region = c.def_region (s, bline, eline, "", "", file);
	cube_node = c.def_cnode (cube_region, file, bline, parent);

	/* Store # occurrences */
	Metric *m = c.get_met (NO_OCCURRENCES);
	Thread *t = (c.get_thrdv())[0];
	c.set_sev (m, cube_node, t, occurrences);
}

void CubeTree::generateSibling (Cube &c, Cnode *parent, CallstackTree *ctree, 
	UIParaverTraceConfig *pcf)
{
	if (ctree != NULL)
	{
		vector<CallstackTree*> vctree = ctree->getChildren();
		if (vctree.size() > 0)
		{
			unsigned CallerSiblingID = 0;
			CubeTree *sibtmp;
			for (unsigned u = 0; u < vctree.size(); u++)
			{
				CodeRefTriplet crt2 = vctree[u]->getNodeTriplet();
				cout << "Children Sibling Triplet <" << crt2.Caller << ", " << crt2.CallerLine << ", " << crt2.CallerLineAST << "> compared to <" << CallerSiblingID << ", ..>" << endl;

				/* Are they siblings? Are they called the same ? */
				if (CallerSiblingID != vctree[u]->getNodeTriplet().Caller)
				{
					CubeTree *sibtmp = new CubeTree;
					sibtmp->generate (c, parent, vctree[u], pcf);
					children.push_back (sibtmp);
					CallerSiblingID = vctree[u]->getNodeTriplet().Caller;
				}
				else
				{
					CubeTree *tmp = new CubeTree;
					tmp->generateSibling (c, cube_node, vctree[u], pcf);
					children.push_back (tmp);
				}
			}
		}
		else
		{
			CodeRefTriplet crt2 = ctree->getNodeTriplet();
			cout << "Children Sibling Leaf <" << crt2.Caller << ", " << crt2.CallerLine << ", " << crt2.CallerLineAST << endl;

			CubeTree *tmp = new CubeTree;
			tmp->generateLeaf (c, parent, ctree->getNodeTriplet(),
			  ctree->getOccurrences(), pcf);
		}
	}
}

void CubeTree::generate (Cube &c, Cnode *parent, CallstackTree *ctree, 
	UIParaverTraceConfig *pcf)
{
	if (ctree != NULL)
	{
		CodeRefTriplet crt = ctree->getNodeTriplet();

		cout << "Parent Triplet <" << crt.Caller << ", " << crt.CallerLine << ", " << crt.CallerLineAST << ">" << endl;

		string routine, file;
		int bline, eline;

		common::lookForFullCallerInfo (pcf, crt.Caller, crt.CallerLineAST, routine,
		  file, bline, eline);

		/* Create a node for this routine */
		cube_region = c.def_region (routine, 0, 0, "", "", file);
		cube_node = c.def_cnode (cube_region, file, 0, parent);
		tree_node = ctree;

		/* Store # occurrences */
		Metric *m = c.get_met (NO_OCCURRENCES);
		Thread *t = (c.get_thrdv())[0];
		c.set_sev (m, cube_node, t, 0);

		vector<CallstackTree*> vctree = ctree->getChildren();
		if (vctree.size() > 0)
		{
			unsigned CallerSiblingID = 0;
			CubeTree *sibtmp;
			for (unsigned u = 0; u < vctree.size(); u++)
			{
				CodeRefTriplet crt2 = vctree[u]->getNodeTriplet();
				cout << "Children Triplet <" << crt2.Caller << ", " << crt2.CallerLine << ", " << crt2.CallerLineAST << "> compared to <" << CallerSiblingID << ", ..>" << endl;

				/* Are they siblings? Are they called the same ? If so, reuse 
				   the information from the first */
				if (CallerSiblingID != vctree[u]->getNodeTriplet().Caller)
				{
					sibtmp = new CubeTree;
					sibtmp->generate (c, cube_node, vctree[u], pcf);
					children.push_back (sibtmp);
					CallerSiblingID = vctree[u]->getNodeTriplet().Caller;
				}
				else
				{
					CubeTree *tmp = new CubeTree;
					tmp->generateSibling (c, sibtmp->cube_node, vctree[u], pcf);
					children.push_back (tmp);
				}
			}
		}
		else
		{
			CodeRefTriplet crt2 = ctree->getNodeTriplet();
			cout << "Children Leaf <" << crt2.Caller << ", " << crt2.CallerLine << ", " << crt2.CallerLineAST << ">" << endl;

			CubeTree *tmp = new CubeTree;
			tmp->generateLeaf (c, cube_node, ctree->getNodeTriplet(),
			  ctree->getOccurrences(), pcf);
		}
	}
}

#else

/* Use this as a reference to compare CUBE trees with & w/o sibling compacting */

void CubeTree::generateLeaf (Cube &c, Cnode *parent, CodeRefTriplet crt,
	unsigned occurrences, UIParaverTraceConfig *pcf)
{
}

void CubeTree::generateSibling (Cube &c, Cnode *parent, CallstackTree *ctree, 
	UIParaverTraceConfig *pcf)
{
}

void CubeTree::generate (Cube &c, Cnode *parent, CallstackTree *ctree, 
	UIParaverTraceConfig *pcf)
{
	if (ctree != NULL)
	{
		CodeRefTriplet crt = ctree->getNodeTriplet();

		string routine, file;
		int bline, eline;

		common::lookForFullCallerInfo (pcf, crt.Caller, crt.CallerLineAST, routine,
		  file, bline, eline);

		stringstream blines, elines;
		blines << bline;
		elines << eline;

		string s = routine + " (" + file;
		if (bline != eline)
			s += " [" + blines.str() + "-" + elines.str() + "])";
		else
			s += " [" + blines.str() + "])";

		/* Create a node for this routine */
		cube_region = c.def_region (s, bline, eline, "", "", file);
		cube_node = c.def_cnode (cube_region, file, 0, parent);
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
				tmp->generate (c, cube_node, vctree[u], pcf);
				children.push_back (tmp);
			}
		}
	}
}


#endif
