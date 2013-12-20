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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/sample.C $
 | 
 | @last_commit: $Date: 2013-10-25 17:11:45 +0200 (Fri, 25 Oct 2013) $
 | @version:     $Revision: 2250 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id: sample.C 2250 2013-10-25 15:11:45Z harald $";

#include "common.H"

#include "sample.H"

#include <iostream>
#include <assert.h>

Sample::Sample (unsigned long long sTime, unsigned long long iTime,
      map<string, unsigned long long> & icountervalue,
      map<unsigned, CodeRefTriplet> & codetriplet)
	: sTime (sTime), iTime(iTime)
{
	this->iCounterValue = icountervalue;
	this->CodeTriplet = codetriplet;
}

Sample::~Sample (void)
{
}

void Sample::normalizeData (unsigned long long instanceDuration,
	map<string, unsigned long long> & instanceCounterValue)
{
	nTime = ((double) iTime / (double) instanceDuration);

	map<string,unsigned long long>::iterator i;
	for (i = iCounterValue.begin(); i != iCounterValue.end(); ++i)
	{
		string counter = (*i).first;
		assert (instanceCounterValue.count(counter) == 1);

		nCounterValue[counter] =
		  ((double) (iCounterValue[counter])) / ((double) instanceCounterValue[counter]);
	}
}

void Sample::show (void)
{
	map<unsigned, CodeRefTriplet>::reverse_iterator it1;
	cout << "Sample @ " << sTime << endl;
	for (it1 = CodeTriplet.rbegin(); it1 != CodeTriplet.rend(); it1++)
		cout << "[ " << (*it1).first << " <" << (*it1).second.getCaller() 
		  << "," << (*it1).second.getCallerLine() << "," 
		  << (*it1).second.getCallerLineAST() << "> ]" << endl;
	map<string, unsigned long long>::iterator it2;
	cout << "[";
	for (it2 = iCounterValue.begin(); it2 != iCounterValue.end(); it2++)
		cout << " " << (*it2).first << "," << (*it2).second;
	cout << " ]" << endl;
}

void Sample::processCodeTriplets (void)
{
	/* Remove head callers 0..2 */
	map<unsigned, CodeRefTriplet>::iterator i = CodeTriplet.begin();
	if ((*i).second.getCaller() <= 2) /* 0 = End, 1 = Unresolved, 2 = Not found */
	{
		do
		{
			CodeTriplet.erase (i);
			if (CodeTriplet.size() == 0)
				break;
			i = CodeTriplet.begin();
		} while ((*i).second.getCaller() <= 2);
	}

	/* Remove tail callers 0..2 */
	i = CodeTriplet.begin();
	bool tailFound = false;
	map<unsigned, CodeRefTriplet>::iterator it;
	while (i != CodeTriplet.end())
	{
		if ((*i).second.getCaller() <= 2 && !tailFound)
		{
			it = i;
			tailFound = true;
		}
		else if ((*i).second.getCaller() > 2)
			tailFound = false;
		i++;
	}
	if (tailFound)
		CodeTriplet.erase (it, CodeTriplet.end());
}

bool Sample::hasCounter (string ctr) const
{
	return iCounterValue.count (ctr) > 0;
}

double Sample::getNCounterValue (string ctr)
{
	assert (hasCounter(ctr));
	return nCounterValue[ctr];
}

unsigned long long Sample::getCounterValue (string ctr)
{
	assert (hasCounter(ctr));
	return iCounterValue[ctr];
}

bool Sample::hasCaller (unsigned caller)
{
	map<unsigned, CodeRefTriplet>::iterator i;
	for (i = CodeTriplet.begin(); i != CodeTriplet.end(); i++)
		if ((*i).second.getCaller() == caller)
			return true;
	return false;
}

set<string> Sample::getCounters (void)
{
	set<string> res;
	map<string, unsigned long long>::iterator it;
	//cout << "getCounters map.size()=" << iCounterValue.size() << endl;
	for (it = iCounterValue.begin(); it != iCounterValue.end(); it++)
	{
	//	cout << "Adding " << (*it).first << endl;
		res.insert ((*it).first);
	}
	return res;
}

