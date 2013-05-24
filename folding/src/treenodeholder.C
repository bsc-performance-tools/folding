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

#include "common.H"
#include "treenodeholder.H"
#include <iostream>
#include <sstream>

TreeNodeHolder * TreeNodeHolder::lookForCallerLine (unsigned id)
{
   TreeNodeHolder *tmp;
   for (unsigned u = 0; u < children.size(); u++)
		if (children[u]->CallerLine == id)
			return this;

   for (unsigned u = 0; u < children.size(); u++)
     if ( (tmp = children[u]->lookForCallerLine (id)) != NULL)
       return tmp;
   return NULL;
}

void TreeNodeHolder::AddPath (unsigned depth, ca_callstacksample &ca, UIParaverTraceConfig *pcf, cube::Cube *cubev, cube::Cnode *root, string &sourceDir)
{
#ifdef DEBUG
	string as = "*";
	string em = " ";

	cout << "ADDPATH depth = " << depth << endl;
	cout << "ADDPATH BEGIN adding samples " << endl;
	for (unsigned u = 0; u < ca.callerline.size(); u++)
		if (u==depth)
			cout << as << " ADDPATH sample " << ca.callerline[u] << endl;
		else
			cout << em << " ADDPATH sample " << ca.callerline[u] << endl;
	cout << "ADDPATH END adding samples " << endl;

	cout << "COMPARING " << ca.callerline[depth] << endl;
#endif

	bool found = false;
	for (unsigned u = 0; u < children.size() && !found; u++)
	{
#ifdef DEBUG
		cout << ".. WITH " << children[u]->CallerLine << endl;
#endif

		if (ca.callerline[depth] == children[u]->CallerLine)
		{
			found = true;
			//children[u]->Occurrences++;
			if (depth+1 < ca.callerline.size())
				children[u]->AddPath (depth+1, ca, pcf, cubev, children[u]->CubeNode, sourceDir);
			else
				children[u]->Occurrences++;
		} 
	}
#ifdef DEBUG
	cout << "END COMPARING " << ca.callerline[depth] << endl;
#endif

	if (!found)
	{
#ifdef DEBUG
		cout << "NOT FOUND BUILDING " << ca.callerline[depth] << endl;
#endif
		TreeNodeHolder *tmp = new TreeNodeHolder;
		tmp->Caller = ca.caller[depth];
		tmp->CallerLine = ca.callerline[depth];
		tmp->Occurrences = (depth+1 < ca.callerline.size())?0:1;

#if !HAVE_CUBE
		tmp->CubeNode = NULL;
#else
		int line;
		string file;
		common::lookForCallerLineInfo (pcf, tmp->CallerLine, file, line);

		string s;
		if (depth+1 < ca.callerline.size())
			s = pcf->getEventValue (30000000, tmp->Caller);
		else
		{
			stringstream s1;
			s1 << line;
			s = pcf->getEventValue (30000000, tmp->Caller) + " " + file + " [" + s1.str() + "-" + s1.str() + "]";
		}

		cube::Region* r = cubev->def_region (s, 0, 0, "", "", "");
		tmp->CubeNode = cubev->def_cnode (r, sourceDir+"/"+file, line, root);
		tmp->CallerLineASTBlock = make_pair (line,line);
#endif

		/* If there are siblings, add sorted by line # (use identifier in PCF) */
		if (children.size() > 0)
		{
			vector<TreeNodeHolder*>::iterator it = children.begin();
			while (tmp->CallerLine > (*it)->CallerLine)
				if (++it == children.end())
					break;
			children.insert (it, tmp);
		}
		else
			children.push_back (tmp);

		if (depth+1 < ca.callerline.size())
			tmp->AddPath (depth+1, ca, pcf, cubev, tmp->CubeNode, sourceDir);
	}
}

void TreeNodeHolder::AddPathAST (unsigned depth, ca_callstacksample &ca, UIParaverTraceConfig *pcf, cube::Cube *cubev, cube::Cnode *root, string &sourceDir)
{
#ifdef DEBUG
	string as = "*";
	string em = " ";

	cout << "ADDPATH depth = " << depth << endl;
	cout << "ADDPATH BEGIN adding samples " << endl;
	for (unsigned u = 0; u < ca.callerlineASTBlock.size(); u++)
		if (u==depth)
			cout << as << " ADDPATH sample " << ca.callerlineASTBlock[u].first << "." << ca.callerlineASTBlock[u].second << endl;
		else
			cout << em << " ADDPATH sample " << ca.callerlineASTBlock[u].first << "." << ca.callerlineASTBlock[u].second << endl;
	cout << "ADDPATH END adding samples " << endl;

	cout << "COMPARING " << ca.callerlineASTBlock[u].first << "." << ca.callerlineASTBlock[u].second << endl;
#endif

	bool found = false;
	for (unsigned u = 0; u < children.size() && !found; u++)
	{
#ifdef DEBUG
		cout << ".. WITH " << children[u]->CallerLine << endl;
#endif

		if (ca.callerlineASTBlock[depth] == children[u]->CallerLineASTBlock)
		{
			found = true;
			//children[u]->Occurrences++;
			if (depth+1 < ca.callerlineASTBlock.size())
				children[u]->AddPathAST (depth+1, ca, pcf, cubev, children[u]->CubeNode, sourceDir);
			else
				children[u]->Occurrences++;
		} 
	}
#ifdef DEBUG
	cout << "END COMPARING " << ca.callerlineASTBlock[depth].first << "." << ca.callerlineASTBlock[depth].second << endl;
#endif

	if (!found)
	{
#ifdef DEBUG
		cout << "NOT FOUND BUILDING " << ca.callerlineASTBlock[depth].first << "." << ca.callerlineASTBlock[depth].second << endl;
#endif
		TreeNodeHolder *tmp = new TreeNodeHolder;
		tmp->Caller = ca.caller[depth];
		tmp->CallerLine = ca.callerline[depth];
		tmp->CallerLineASTBlock = ca.callerlineASTBlock[depth];
		tmp->Occurrences = (depth+1 < ca.callerlineASTBlock.size())?0:1;

#if !HAVE_CUBE
		tmp->CubeNode = NULL;
#else

		int line;
		string file;
		common::lookForCallerLineInfo (pcf, tmp->CallerLine, file, line);

		string s;
		if (depth+1 < ca.callerlineASTBlock.size())
		{
			s = pcf->getEventValue (30000000, tmp->Caller);
		}
		else
		{
			stringstream s1, s2;
			s1 << ca.callerlineASTBlock[depth].first;
			s2 << ca.callerlineASTBlock[depth].second;
			s = pcf->getEventValue (30000000, tmp->Caller) + " " + file + " [" + s1.str() + "-" + s2.str() + "]";
		}

		cube::Region* r = cubev->def_region (s, ca.callerlineASTBlock[depth].first, ca.callerlineASTBlock[depth].second, "", "", sourceDir+"/"+file);

		//for (unsigned u = ca.callerlineASTBlock[depth].first; u < ca.callerlineASTBlock[depth].second; u++)
			tmp->CubeNode = cubev->def_cnode (r, sourceDir+"/"+file, ca.callerlineASTBlock[depth].first, root);
#endif

		/* If there are siblings, add sorted by line # (use identifier in PCF) */
		if (children.size() > 0)
		{
			vector<TreeNodeHolder*>::iterator it = children.begin();
			while (tmp->CallerLine > (*it)->CallerLine)
				if (++it == children.end())
					break;
			children.insert (it, tmp);
		}
		else
			children.push_back (tmp);

		if (depth+1 < ca.callerlineASTBlock.size())
			tmp->AddPath (depth+1, ca, pcf, cubev, tmp->CubeNode, sourceDir);
	}
}
