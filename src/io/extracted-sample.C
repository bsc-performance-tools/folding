
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

static char __attribute__ ((unused)) rcsid[] = "$Id: read-extracted-data.C 2257 2013-10-29 11:09:15Z harald $";

#include "common.H"

#include "extracted-sample.H"

#include <assert.h>

extractedSample::extractedSample (unsigned long long TS,
	const map<string, unsigned long long> & CV,
	const map<unsigned, unsigned long long> & C,
	const map<unsigned, unsigned long long> & CL,
	const map<unsigned, unsigned long long> & CLA)
	: Timestamp (TS), CounterValues (CV), Caller (C), CallerLine (CL),
	  CallerLineAST(CLA)
{
}

set<string> extractedSample::getCounters (void)
{
	set<string> res;
	map<string, unsigned long long>::iterator it = CounterValues.begin();
	for (; it != CounterValues.end(); it++)
		res.insert ((*it).first);
}

unsigned long long extractedSample::getCounterValue (string C)
{
	assert (CounterValues.count(C) > 0);
	return CounterValues[C];
}

