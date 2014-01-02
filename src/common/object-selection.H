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

#ifndef OBJECT_SELECTION_H_INCLUDED
#define OBJECT_SELECTION_H_INCLUDED

#include <string>

using namespace std;

class ObjectSelection
{
	private:
	const unsigned ptask;
	const unsigned task;
	const unsigned thread;
	const bool anyptask;
	const bool anytask;
	const bool anythread;

	public:
	ObjectSelection ();
	ObjectSelection (unsigned ptask, bool aptask, unsigned task, bool atask,
	  unsigned thread, bool athread);
	ObjectSelection (unsigned ptask, unsigned task, unsigned thread);

	const bool operator< (ObjectSelection param) const
	{
		if (anyptask || anytask || anythread ||
		    param.anyptask || param.anytask ||  param.anythread)
		return false;

		if (ptask < param.ptask)
			return true;
		else if (task < param.task)
			return true;
		else
			return thread < param.thread;
	}

	bool match (unsigned ptask, unsigned task, unsigned thread) const
	  { return (this->anyptask || this->ptask == ptask) &&
	           (this->anytask || this->task == task) &&
	           (this->anythread || this->thread == thread); }

	bool anyany (void) const
	  { return this->anyptask || this->anytask || this->anythread; }

	string toString (bool prefix = false, const string & anytext = "*") const;

	unsigned getptask (void) const
	  { return ptask; }

	unsigned gettask (void) const
	  { return task; }

	unsigned getthread (void) const
	  { return thread; }
};

#endif /* OBJECT_SELECTION_H_INCLUDED */
