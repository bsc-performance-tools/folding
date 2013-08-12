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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/callstackanalysis.C $
 | 
 | @last_commit: $Date: 2013-05-24 16:08:28 +0200 (dv, 24 mai 2013) $
 | @version:     $Revision: 1764 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id: callstackanalysis.C 1764 2013-05-24 14:08:28Z harald $";

#include "common.H"

#include "sample.H"

#include <iostream>
#include <assert.h>

Sample::Sample (unsigned long long sTime, unsigned long long iTime, double nTime,
      map<string, unsigned long long> & icountervalue,
      map<string, double> & ncountervalue,
      map<unsigned, CodeRefTriplet> & codetriplet)
	: sTime (sTime), iTime(iTime), nTime (nTime)
{
	this->nCounterValue = ncountervalue;
	this->iCounterValue = icountervalue;
	this->CodeTriplet = codetriplet;
}

Sample::~Sample (void)
{
}

void Sample::show (void)
{
	map<unsigned, CodeRefTriplet>::reverse_iterator it1;
	for (it1 = CodeTriplet.rbegin(); it1 != CodeTriplet.rend(); it1++)
		cout << "Sample [" << (*it1).first << " <" << (*it1).second.getCaller() 
		  << "," << (*it1).second.getCallerLine() << "," 
		  << (*it1).second.getCallerLineAST() << "> ] @ " << sTime;
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
	return nCounterValue.count (ctr) > 0;
}

double Sample::getNCounterValue (string ctr)
{
	assert (hasCounter(ctr));
	return nCounterValue[ctr];
}

bool Sample::hasCaller (unsigned caller)
{
	map<unsigned, CodeRefTriplet>::iterator i;
	for (i = CodeTriplet.begin(); i != CodeTriplet.end(); i++)
		if ((*i).second.getCaller() == caller)
			return true;
	return false;
}
