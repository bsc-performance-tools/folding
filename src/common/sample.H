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

#ifndef SAMPLE_H_INCLUDED
#define SAMPLE_H_INCLUDED

#include <map>
#include <set>
#include <string>

#include "codereftriplet.H"

using namespace std;

class Sample
{
	private:
	const unsigned long long sTime; /* integral time of the sample */
	const unsigned long long iTime; /* integral time since start of instance */
	double nTime; /* normalized time since start of instance */
	map<string, unsigned long long> iCounterValue; /* integral counter value */
	map<string, double> nCounterValue; /* normalized counter value */
	map<unsigned, CodeRefTriplet> CodeTriplet; /* Code reference triplets (caller, callerline, callerlineAST) */
	const bool bhasAddressReference;           /* Does this sample point to a data address? */
	const unsigned long long AddressReference; /* which address? */
	const unsigned AddressReference_Mem_Level; /* where was it found in mem hierarchy ? */
	const unsigned AddressReference_TLB_Level; /* where was it found in tlb hierarchy ? */
	const unsigned AddressReference_Cycles_Cost; /* how many cycles to resolve this reference */
	public:

	/* sTime = sample time within the full exec, iTime time since instance */
	Sample (unsigned long long sTime, unsigned long long iTime,
	  map<string, unsigned long long> & icountervalue,
	  map<unsigned, CodeRefTriplet> & codetriplet);

	Sample (unsigned long long sTime, unsigned long long iTime,
	  map<string, unsigned long long> & icountervalue,
	  map<unsigned, CodeRefTriplet> & codetriplet,
	  unsigned long long address, unsigned ar_mem_level,
	  unsigned ar_tlb_level, unsigned cycles_cost);

	~Sample (void);

	void normalizeData (unsigned long long instaceDuration,
	  map<string, unsigned long long> & instanceCounterValue,
	  const string & TimeUnit);
	void processCodeTriplets (void);
	void show (void);
	unsigned long long getTime (void) const
	  { return sTime; }
	unsigned long long getiTime (void) const
	  { return iTime; }
	double getNTime (void) const
	  { return nTime; }
	bool hasCounter (string ctr) const;
	bool hasCounters (void) const
	  { return iCounterValue.size() > 0; }
	double getNCounterValue (string ctr);
	unsigned long long getCounterValue (string ctr);
	map<string, double> getNCounterValue(void)
	  { return nCounterValue; }
	map<unsigned, CodeRefTriplet> getCodeTriplets(void)
	  { return CodeTriplet; }
	bool hasCaller (unsigned caller);
	unsigned getCodeRefTripletSize(void) const
	  { return CodeTriplet.size(); }
	unsigned hasCodeRefTripletSize(void) const
	  { return CodeTriplet.size() > 0; }
	set<string> getCounters (void);

	bool hasAddressReference(void) const
	  { return bhasAddressReference; }
	bool hasAddressReferenceInStack(void) const
	  { return AddressReference >= 0x80000000; }
	unsigned long long getAddressReference(void) const
	  { return AddressReference; }
	unsigned getAddressReference_Mem_Level(void) const
	  { return AddressReference_Mem_Level; }
	unsigned getAddressReference_TLB_Level(void) const
	  { return AddressReference_TLB_Level; }
	unsigned getAddressReference_Cycles_Cost(void) const
	  { return AddressReference_Cycles_Cost; }
};

#endif

