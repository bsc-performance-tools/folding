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
#ifndef SAMPLE_H_INCLUDED
#define SAMPLE_H_INCLUDED

#include <map>
#include <set>
#include <string>

#include "callstack-codereftriplet.H"

using namespace std;

enum AddressReferenceType_t { NONE, LOAD, STORE };

class Sample
{
	private:
	const unsigned long long sTime; /* integral time of the sample */
	const unsigned long long iTime; /* integral time since start of instance */
	const bool bhasAddressReference;           /* Does this sample point to a data address? */
	const bool bhasAddressReferenceInfo;       /* Does this sample contain information from data reference? */
	const AddressReferenceType_t ReferenceType; /* Load or Store? */
	const unsigned long long AddressReference; /* which address? */
	const unsigned long long AddressReference_AllocatedObject; /* Allocated object id */
	const unsigned AddressReference_Mem_Level; /* where was it found in mem hierarchy ? */
	const unsigned AddressReference_TLB_Level; /* where was it found in tlb hierarchy ? */
	const unsigned AddressReference_Cycles_Cost; /* how many cycles to resolve this reference */

	double nTime; /* normalized time since start of instance */
	map<string, unsigned long long> iCounterValue; /* integral counter value */
	map<string, double> nCounterValue; /* normalized counter value */
	bool usableCallstack; /* Is the callstack marked as valid / useful? */
	Callstack_CodeRefTriplet crt; /* Code reference triplets (caller, callerline, callerlineAST) */
	unsigned callersId; /* Calculated from outside, in reference to which callers are within this sample */

	public:

	/* sTime = sample time within the full exec, iTime time since instance */
	Sample (unsigned long long sTime, unsigned long long iTime,
	  map<string, unsigned long long> & icountervalue,
	  map<unsigned, CodeRefTriplet> & codetriplet);

	Sample (unsigned long long sTime, unsigned long long iTime,
	  map<string, unsigned long long> & icountervalue,
	  map<unsigned, CodeRefTriplet> & codetriplet,
	  AddressReferenceType_t rt, unsigned long long address,
	  unsigned long long ar_allocatedobject,
	  unsigned ar_mem_level, unsigned ar_tlb_level, unsigned cycles_cost);

	Sample (unsigned long long sTime, unsigned long long iTime,
	  map<string, unsigned long long> & icountervalue,
	  map<unsigned, CodeRefTriplet> & codetriplet,
	  AddressReferenceType_t rt, unsigned long long address,
	  unsigned long long ar_allocatedobject);

	~Sample (void);

	const Callstack_CodeRefTriplet & getCallstackCodeRefTriplet (void)
	  { return crt; }
	void setUsableCallstack (bool usable)
	  { usableCallstack = usable; }
	bool getUsableCallstack (void) const
	  { return usableCallstack; }
	void addCallstackBubbles (unsigned nbubbles)
	  { crt.addBubbles (nbubbles); }
	void setCallstackMaxDepth (unsigned max)
	  { crt.setMaxDepth (max); }
	void copyBottomStack (Sample *other)
	  { crt.copyBottomStack (other->getCallstackCodeRefTriplet()); };

	void normalizeData (unsigned long long instaceDuration,
	  map<string, unsigned long long> & instanceCounterValue,
	  const string & TimeUnit);
	void processCodeTriplets (void);
	void show (bool counters = true);
	unsigned long long getTime (void) const
	  { return sTime; }
	unsigned long long getiTime (void) const
	  { return iTime; }
	double getNTime (void) const
	  { return nTime; }
	bool hasCounter (string ctr) const;
	bool hasCounters (void) const
	  { return iCounterValue.size() > 0; }
	double getNCounterValue (string ctr) const;
	unsigned long long getCounterValue (string ctr) const;
	map<string, double> getNCounterValue(void)
	  { return nCounterValue; }
	map<unsigned, CodeRefTriplet> getCodeTriplets(void)
	  { return crt.getAsMap(); }
	const map<unsigned, CodeRefTriplet> & getCodeTripletsAsConstReference (void) const
	  { return crt.getAsConstReferenceMap(); }
	bool hasCaller (unsigned caller) const;
	unsigned getCallerLevel (unsigned caller) const;
	unsigned getMaxCallerLevel (void) const;
	unsigned getCodeRefTripletSize(void) const
	  { return crt.getSize(); }
	unsigned hasCodeRefTripletSize(void) const
	  { return crt.getSize() > 0; }
	set<string> getCounters (void) const;

	bool hasAddressReference(void) const
	  { return bhasAddressReference; }
	bool hasAddressReferenceInfo(void) const
	  { return bhasAddressReferenceInfo; }
	unsigned long long getAddressReference(void) const
	  { return AddressReference; }
	unsigned long long getAddressReference_AllocatedObject(void) const
	  { return AddressReference_AllocatedObject; }
	unsigned getAddressReference_Mem_Level(void) const
	  { return AddressReference_Mem_Level; }
	unsigned getAddressReference_TLB_Level(void) const
	  { return AddressReference_TLB_Level; }
	unsigned getAddressReference_Cycles_Cost(void) const
	  { return AddressReference_Cycles_Cost; }
	AddressReferenceType_t getReferenceType (void) const
	  { return ReferenceType; }
	

	void setCallersId (unsigned id)
	  { callersId = id; }
	unsigned getCallersId (void) const
	  { return callersId; }
};

#endif

