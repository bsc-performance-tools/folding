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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/sample.H $
 | 
 | @last_commit: $Date: 2013-10-25 17:11:45 +0200 (Fri, 25 Oct 2013) $
 | @version:     $Revision: 2250 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CALLSTACK_PROCESSOR_CONSECUTIVE_RECURSIVE_CTRL_H_INCLUDED
#define CALLSTACK_PROCESSOR_CONSECUTIVE_RECURSIVE_CTRL_H_INCLUDED

#include <deque>

#include "codereftriplet.H"

class CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl
{
	private:
	const unsigned nConsecutiveSamples;

	/* keep callers in the deque plus their timings */
	deque<pair<CodeRefTriplet,double>> callers_times;

	public:
	CallstackProcessor_ConsecutiveRecursive_ConsecutiveCtrl (
	  unsigned nConsecutiveSamples);
	void add (const pair<CodeRefTriplet,double> & callertime);
	bool allEqual (void) const;
	void show (void) const;
	double getFirstTime(void) const;
	double getLastTime(void) const;
	CodeRefTriplet getFirstCodeRef(void) const;
};

#endif /* CALLSTACK_PROCESSOR_CONSECUTIVE_RECURSIVE_CTRL_H_INCLUDED */
