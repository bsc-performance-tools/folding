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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/common.C $
 | 
 | @last_commit: $Date: 2013-05-24 16:08:28 +0200 (dv, 24 mai 2013) $
 | @version:     $Revision: 1764 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef SAMPLE_H_INCLUDED
#define SAMPLE_H_INCLUDED

#include "map"

#include "codereftriplet.H"

using namespace std;

class Sample
{
	private:
	const unsigned long long sTime; /* integral time of the sample */
	const unsigned long long iTime; /* integral time since start of instance */
	const double nTime; /* normalized time since start of instance */
	map<string, unsigned long long> iCounterValue; /* integral counter value */
	map<string, double> nCounterValue; /* normalized counter value */
	map<unsigned, CodeRefTriplet> CodeTriplet; /* Code reference triplets (caller, callerline, callerlineAST) */

	public:
	Sample (unsigned long long sTime, unsigned long long iTime, double nTime,
	  map<string, unsigned long long> & icountervalue,
	  map<string, double> & ncountervalue,
	  map<unsigned, CodeRefTriplet> & codetriplet);
	~Sample (void);

	void processCodeTriplets (void);
	void show (void);
	double getNTime (void) const
	  { return nTime; }
	bool hasCounter (string ctr) const;
	double getNCounterValue (string ctr);
	map<string, double> getNCounterValue(void)
	  { return nCounterValue; }
	map<unsigned, CodeRefTriplet> getCodeTriplets(void)
	  { return CodeTriplet; }
	bool hasCaller (unsigned caller);
	unsigned getCodeRefTripletSize(void) const
	  { return CodeTriplet.size(); }
};

#endif

