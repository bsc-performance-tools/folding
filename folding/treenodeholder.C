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

#include "treenodeholder.H"

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

void TreeNodeHolder::AddPath (unsigned depth, ca_callstacksample &ca)
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

//#define ALL_CALLERLINES

#ifndef ALL_CALLERLINES
		if ((ca.callerline.size()-1 == depth && ca.callerline[depth] == children[u]->CallerLine) ||
		    (ca.callerline.size()-1 != depth && ca.caller[depth] == children[u]->Caller))
#else
		if (ca.callerline[depth] == children[u]->CallerLine)
#endif

		{
			found = true;
			//children[u]->Occurrences++;
			if (depth+1 < ca.callerline.size())
				children[u]->AddPath (depth+1, ca);
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
			tmp->AddPath (depth+1, ca);
	}
}

