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

#ifndef CALLSTACK_PROCESSOR_H_INCLUDED
#define CALLSTACK_PROCESSOR_H_INCLUDED

#include "sample.H"

#include <vector>
#include <utility>

// #include "instance-group.H"
class InstanceGroup; /* There is a circular dependency here, avoid including instance-group.H */

using namespace std;

class CallstackProcessor_Result
{
	private:
	const unsigned Level;
	const CodeRefTriplet CodeRef;
	const double Time;

	public:
	CallstackProcessor_Result (unsigned level, CodeRefTriplet coderef,
	  double time);

	unsigned getLevel (void) const
	  { return Level; }
	CodeRefTriplet getCodeRef (void) const
	  { return CodeRef; }
	double getNTime (void) const
	  { return Time; }
};

class CallstackProcessor
{
	protected:
	const InstanceGroup *ig;

	public:
	CallstackProcessor (InstanceGroup *ig);	
	virtual vector < CallstackProcessor_Result* > processSamples (
	  const vector<Sample*> & samples) = 0;
	virtual ~CallstackProcessor();
};

#endif /* CALLSTACK_PROCESSOR_H_INCLUDED */
