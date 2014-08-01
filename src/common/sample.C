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

#include "common.H"
#include "sample.H"

#include <iostream>
#include <assert.h>

Sample::Sample (unsigned long long sTime, unsigned long long iTime,
    map<string, unsigned long long> & icountervalue,
    map<unsigned, CodeRefTriplet> & codetriplet)
	: sTime (sTime), iTime(iTime), bhasAddressReference(false),
	  AddressReference(0), AddressReference_Mem_Level(0),
	  AddressReference_TLB_Level(0), AddressReference_Cycles_Cost(0),
	  crt(codetriplet)
{
	this->iCounterValue = icountervalue;
	this->usableCallstack = false;
}

Sample::Sample (unsigned long long sTime, unsigned long long iTime,
	map<string, unsigned long long> & icountervalue,
	map<unsigned, CodeRefTriplet> & codetriplet,
	unsigned long long address, unsigned ar_mem_level, unsigned ar_tlb_level,
	unsigned cycles_cost)
	: sTime (sTime), iTime(iTime), bhasAddressReference(true),
	  AddressReference(address),
	  AddressReference_Mem_Level(ar_mem_level),
	  AddressReference_TLB_Level(ar_tlb_level),
	  AddressReference_Cycles_Cost(cycles_cost),
	  crt(codetriplet)
{
	this->iCounterValue = icountervalue;
	this->usableCallstack = false;
}

Sample::~Sample (void)
{
}

void Sample::normalizeData (unsigned long long instanceDuration,
	map<string, unsigned long long> & instanceCounterValue,
	const string & TimeUnit)
{
	assert (common::DefaultTimeUnit == TimeUnit || instanceCounterValue.count(TimeUnit) > 0);

	for (const auto & counterpair : iCounterValue)
	{
		string counter = counterpair.first;
		assert (instanceCounterValue.count(counter) == 1);

		nCounterValue[counter] =
		  ((double) (iCounterValue[counter])) / ((double) instanceCounterValue[counter]);
	}

	if (common::DefaultTimeUnit != TimeUnit)
		nTime = nCounterValue[TimeUnit];
	else
		nTime = ((double) iTime) / ((double) instanceDuration);
}

void Sample::show (bool counters)
{
	if (counters)
	{
		cout << "Sample @ " << sTime << endl;
	
		/* Show callers first */
		crt.show (true);

		map<string, unsigned long long>::const_iterator it2;
		cout << "[";
		for (const auto & counterpair : iCounterValue)
			cout << " " << counterpair.first << "," << counterpair.second;
		cout << " ]" << endl;
	}
	else
	{
		/* One-liner approach*/

		cout << "Sample @ " << sTime << " ";
		crt.show (true);
	}
}

void Sample::processCodeTriplets (void)
{
	crt.processCodeTriplets ();
}

bool Sample::hasCounter (string ctr) const
{
	return iCounterValue.count (ctr) > 0;
}

double Sample::getNCounterValue (string ctr) const
{
	assert (hasCounter(ctr));
	map<string, double>::const_iterator i = nCounterValue.find (ctr);
	return (*i).second;
}

unsigned long long Sample::getCounterValue (string ctr) const
{
	assert (hasCounter(ctr));
	map<string, unsigned long long>::const_iterator i = iCounterValue.find (ctr);
	return (*i).second;
}

bool Sample::hasCaller (unsigned caller) const
{
	for (const auto & codereftriplet : crt.getAsConstReferenceMap ())
		if (codereftriplet.second.getCaller() == caller)
			return true;
	return false;
}

unsigned Sample::getCallerLevel (unsigned caller) const
{
#if 1
	assert (hasCaller(caller));
	for (const auto & codereftriplet : crt.getAsConstReferenceMap ())
		if (codereftriplet.second.getCaller() == caller)
			return codereftriplet.first;

	return 0;
#else
	unsigned level = 0;

	assert (hasCaller(caller));
	for (const auto & codereftriplet : crt.getAsConstReferenceMap ())
		if (codereftriplet.second.getCaller() == caller)
			level = MAX(level, codereftriplet.first);

	return level;
#endif
}

unsigned Sample::getMaxCallerLevel (void) const
{
	const map<unsigned, CodeRefTriplet> & ccrt = crt.getAsConstReferenceMap ();
	map<unsigned, CodeRefTriplet>::const_reverse_iterator it = ccrt.crbegin();
	return (*it).first;
}

set<string> Sample::getCounters (void) const
{
	set<string> res;
	for (const auto & counterpair : iCounterValue)
		res.insert (counterpair.first);
	return res;
}

