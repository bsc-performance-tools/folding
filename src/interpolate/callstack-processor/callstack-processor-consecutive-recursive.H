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

#ifndef CALLSTACK_PROCESSOR_CONSECUTIVE_RECURSIVE_H_INCLUDED
#define CALLSTACK_PROCESSOR_CONSECUTIVE_RECURSIVE_H_INCLUDED

#include "callstack-processor.H"

class CallstackProcessor_ConsecutiveRecursive_ProcessedInfo
{
	private:
	const unsigned level;
	const CodeRefTriplet CodeRef;
	const double time;
	const bool speculated;

	public:
	CallstackProcessor_ConsecutiveRecursive_ProcessedInfo (unsigned level,
	  CodeRefTriplet coderef, double time, bool speculated);

	unsigned getLevel (void) const
	  { return level; }
	CodeRefTriplet getCodeRef (void) const
	  { return CodeRef; }
	double getTime (void) const
	  { return time; }
	bool getSpeculated (void) const
	  { return speculated; }
};

class CallstackProcessor_ConsecutiveRecursive : public CallstackProcessor
{
	private:
	const unsigned nConsecutiveSamples;
	const double openRecursion;

	vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > processSamples_r (
	  const vector<Sample*> &samples, unsigned stackdepth, unsigned depth, 
	  double start, double end, bool allow_speculated);

	vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > searchConsecutiveRegions (
	  const vector<Sample*> &samples, unsigned stackdepth, double start,
	  double end, bool allow_speculated);

	vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > searchConsecutiveRegions_speculated (
	  const vector<Sample*> &samples, unsigned stackdepth, double start,
	  double end);

	vector < CallstackProcessor_ConsecutiveRecursive_ProcessedInfo * > searchConsecutiveRegions_nonspeculated (
	  const vector<Sample*> &samples, unsigned stackdepth, double start,
	  double end);

	CodeRefTriplet SearchMostFrequent (const map<CodeRefTriplet, unsigned> & f);

	public:
	CallstackProcessor_ConsecutiveRecursive (InstanceGroup *ig,
	  unsigned nConsecutiveSamples, double openRecursion);
	CallstackProcessor_ConsecutiveRecursive (InstanceGroup *ig,
	  unsigned nConsecutiveSamples, unsigned long long duration);
	
	vector < CallstackProcessor_Result* > processSamples (
	  const vector<Sample*> & samples);
};

#endif /* CALLSTACK_PROCESSOR_CONSECUTIVE_RECURSIVE_H_INCLUDED */
