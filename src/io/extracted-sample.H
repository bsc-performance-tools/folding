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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/read-extracted-data.C $
 | 
 | @last_commit: $Date: 2013-10-29 12:09:15 +0100 (Tue, 29 Oct 2013) $
 | @version:     $Revision: 2257 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef EXTRACTED_SAMPLE_H_INCLUDED
#define EXTRACTED_SAMPLE_H_INCLUDED

#include <map>
#include <set>
#include <string>

using namespace std;

class extractedSample
{
	private:
	const unsigned long long Timestamp;
	map<string, unsigned long long> CounterValues; /* Map to ID of counter to its value */
	map<unsigned, unsigned long long> Caller;        /* Map depth of caller */
	map<unsigned, unsigned long long> CallerLine;    /* Map depth of caller line */
	map<unsigned, unsigned long long> CallerLineAST; /* Map depth of caller line AST */

	public:
	extractedSample (unsigned long long Timestamp,
	  const map<string, unsigned long long> &CV,
	  const map<unsigned, unsigned long long> &C,
	  const map<unsigned, unsigned long long> &CL,
	  const map<unsigned, unsigned long long> &CLA);
	set<string> getCounters (void);
	unsigned long long getCounterValue (string C);
	unsigned long long getTimestamp (void) const
	  { return Timestamp; }
	map<unsigned, unsigned long long> getCaller(void) const
	  { return Caller; }
	map<unsigned, unsigned long long> getCallerLine(void) const
	  { return CallerLine; }
	map<unsigned, unsigned long long> getCallerLineAST(void) const
	  { return CallerLineAST; }
};

#endif /* EXTRACTED_SAMPLE_H_INCLUDED */
