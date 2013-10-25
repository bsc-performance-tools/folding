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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/interpolate.C $
 | 
 | @last_commit: $Date: 2013-07-03 13:58:01 +0200 (Wed, 03 Jul 2013) $
 | @version:     $Revision: 1862 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef PRV_SEMANTIC_CSV_H_INCLUDED
#define PRV_SEMANTIC_CSV_H_INCLUDED

#include "common.H"
#include "prv-semantic.H"
#include "object-selection.H"

#include <string>
#include <vector>
#include <map>

using namespace std;

class PRVSemanticCSV
{
	public:
	PRVSemanticCSV (char *file);

	vector<PRVSemanticValue*> getSemantics (const ObjectSelection &o);
	vector<PRVSemanticValue*> getSemantics (unsigned ptask, unsigned task, unsigned thread);

	bool getSucceeded(void) const
	  { return succeeded; }

	private:
	map<ObjectSelection, vector<PRVSemanticValue*> > SemanticsPerObject;
	bool succeeded;
};

#endif // PRV_SEMANTIC_CSV_H_INCLUDED

