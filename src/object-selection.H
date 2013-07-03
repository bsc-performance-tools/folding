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

#ifndef OBJECT_SELECTION_H_INCLUDED
#define OBJECT_SELECTION_H_INCLUDED

#include <string>

using namespace std;

class ObjectSelection
{
	public:
	unsigned ptask, task, thread;
	bool anyptask, anytask, anythread;

	ObjectSelection ()
	  { anyptask = anytask = anythread = true; }

	ObjectSelection (unsigned ptask, bool aptask, unsigned task, bool atask,
	  unsigned thread, bool athread)
	  { this->ptask = ptask; this->task = task; this->thread = thread;
	    this->anyptask = aptask; this->anytask = atask; this->anythread = athread; }

	ObjectSelection (unsigned ptask, unsigned task, unsigned thread)
	  { this->ptask = ptask; this->task = task; this->thread = thread;
	    this->anyptask = this->anytask = this->anythread = false; }

	bool match (unsigned ptask, unsigned task, unsigned thread)
	  { return (this->anyptask || this->ptask == ptask) &&
	           (this->anytask || this->task == task) &&
	           (this->anythread || this->thread == thread); }

	bool anyany (void)
	  { return this->anyptask || this->anytask || this->anythread; }

	string toString (bool prefix = false, string anytext = "*");
};

#endif /* OBJECT_SELECTION_H_INCLUDED */
