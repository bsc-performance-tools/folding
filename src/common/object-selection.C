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

#include "object-selection.H"

#include <sstream>

ObjectSelection::ObjectSelection () : anyptask (true), anytask(true),
	anythread(true), ptask(0), task(0), thread(0)
{
}

ObjectSelection::ObjectSelection (unsigned ptask, bool aptask, unsigned task,
	bool atask, unsigned thread, bool athread) : anyptask (aptask), anytask(atask),
	anythread(athread), ptask(ptask), task(task), thread(thread)
{
}

ObjectSelection::ObjectSelection (unsigned ptask, unsigned task, unsigned thread) :
	anyptask(false), anytask(false), anythread(false), ptask(ptask), task(task),
	thread(thread)
{
}

string ObjectSelection::toString (bool prefix, string anytext) const
{
	stringstream ss;

	if (prefix)
		ss << "Appl ";
	if (anyptask)
		ss << anytext;
	else
		ss << ptask;
	if (!prefix)
		ss << ".";

	if (prefix)
		ss << " Task ";
	if (anytask)
		ss << anytext;
	else
		ss << task;
	if (!prefix)
		ss << ".";

	if (prefix)
		ss << " Thread ";
	if (anythread)
		ss << anytext;
	else
		ss << thread;

	return ss.str();
}

