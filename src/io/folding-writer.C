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

#include "folding-writer.H"
#include <iostream>
#include <fstream>
#include <assert.h>

bool FoldingWriter::equalTypes (set<string> &m1, set<string> m2)
{
	if (m1.size() != m2.size())
		return false;

	set<string>::iterator i1 = m1.begin();
	set<string>::iterator i2 = m2.begin();

	/* Check all types in M1 are in M2 in the same order */
	for (; i1 != m1.end(); i1++, i2++)
		if (*i1 != *i2)
			return false;

	/* If we are here, then types(M1) = types(M2) */
	return true;
}

bool FoldingWriter::checkSamples (vector<Sample*> &Samples)
{
	/* Triplets are not valid at the edges (first and last) */
	if (Samples.size() > 2)
	{
		set<string> reference = Samples[0]->getCounters();

		/* Just check for counter types, that are the same across the instance.
		   We don't need to check for the callers because they may vary from
		   execution to execution (i.e. one sample may get 10 callers but the
		   following can take less). */
		for (unsigned u = 2; u < Samples.size(); u++)
			if (!equalTypes (reference, Samples[u]->getCounters()))
				return false;
	}

	return true;
}

void FoldingWriter::Write (ofstream &ofile, const string & RegionName,
	unsigned ptask, unsigned task, unsigned thread,
	unsigned long long start, unsigned long long duration,
	vector<Sample*> &Samples)
{
#if 0
	if (common::DEBUG())
		cout << "Process:dumpSamples (Samples.size() = " << Samples.size() << ")" << endl;
#endif

	/* At least, have a sample at the begin & end, and someone else */
	if (Samples.size() < 2)
		return;

	if (!checkSamples (Samples))
		return;

	/* Write the total time spent in this region */

	/* Calculate totals */
	map<string, unsigned long long> totals;
	set<string> Counters = Samples[0]->getCounters();
	set<string>::iterator it;
	for (it = Counters.begin(); it != Counters.end(); ++it)
		totals[*it] = 0; // has to accumulate

	/* Generate header for this instance */
	ofile << "I " << ptask+1 << " " << task+1 << " " << thread+1 
	  << " " << RegionName << " " << start << " " << duration;
	for (unsigned u = 0; u < Samples.size(); u++)
		for (it = Counters.begin(); it != Counters.end(); it++)
			totals[*it] = totals[*it] + Samples[u]->getCounterValue (*it);

	ofile << " " << totals.size();
	map<string, unsigned long long>::iterator it_totals;
	for (it_totals = totals.begin(); it_totals != totals.end(); ++it_totals)
		ofile << " " << (*it_totals).first << " " << (*it_totals).second;
	ofile << endl;

	/* Do not emit samples if we only have a sample at the end */
	if (Samples[0]->getTime() == start + duration)
		return;

	/* Prepare partial accum hash */
	map<string, unsigned long long> partials;
	for (it = Counters.begin(); it != Counters.end(); it++)
		partials[*it] = 0;

	/* Dump all the samples for this instance except the last one, which can be extrapolated
       from the instance header data */
	for (unsigned u = 0; u < Samples.size() - 1; ++u)
	{
		ofile << "S " << Samples[u]->getTime() << " "
		  << Samples[u]->getiTime() << " " << Counters.size();

		for (it = Counters.begin(); it != Counters.end(); it++)
		{
			unsigned long long tmp = partials[*it] +
			  Samples[u]->getCounterValue (*it);
			ofile << " " << *it << " " << tmp;
			partials[*it] = tmp;
		}

		/* Dump callers, callerlines and caller line ASTs triplets */
		map<unsigned, CodeRefTriplet> CodeRefs = Samples[u]->getCodeTriplets();

		ofile << " " << CodeRefs.size();

		map<unsigned, CodeRefTriplet>::iterator cit;
		for (cit = CodeRefs.begin(); cit != CodeRefs.end(); ++cit)
		{
			CodeRefTriplet cr = (*cit).second;

			ofile << " " << (*cit).first << " " << cr.getCaller() 
			  << " " << cr.getCallerLine() << " " <<
			  cr.getCallerLineAST();
		}

		ofile << endl;
	}
}
