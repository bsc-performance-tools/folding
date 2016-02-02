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

#include "folding-writer.H"
#include <iostream>
#include <fstream>
#include <assert.h>

bool FoldingWriter::equalTypes (const set<string> &m1, const set<string> m2)
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

bool FoldingWriter::containTypes (const set<string> &m1, const set<string> m2)
{
	if (m1.size() > m2.size())
		return false;

	/* Check all types in M1 are in M2 */
	for (const auto & s1 : m1)
		if (m2.find (s1) == m2.end())
		{
			if (common::DEBUG())
				cout << "Cannot find counter " << s1 << " in second set" << endl;
			return false;
		}

	/* If we are here, then types(M1) = types(M2) */
	return true;
}

unsigned FoldingWriter::getReferenceSample (const vector<Sample*> &Samples)
{
	unsigned reference = 0;
	bool found = Samples[reference]->hasCounters();
	while (!found && reference < Samples.size())
	{
		reference++;
		if (reference < Samples.size())
			found = Samples[reference]->hasCounters();
	}
	return (found)?reference:0;
}

bool FoldingWriter::checkSamples (const vector<Sample*> &Samples)
{
	/* Triplets are not valid at the edges (first and last) */
	if (Samples.size() > 2)
	{
		unsigned reference = getReferenceSample (Samples);

		set<string> reference_counters = Samples[reference]->getCounters();

		/* Just check for counter types, that are the same across the instance.
		   We don't need to check for the callers because they may vary from
		   execution to execution (i.e. one sample may get 10 callers but the
		   following can take less). */
		for (unsigned u = 2; u < Samples.size(); u++)
		{
			set<string> counters = Samples[u]->getCounters();

			/* If the sample does not have counters (as for address samples), skip it */
			if (counters.size() == 0)
				continue;

			/* Compare counters */
			//if (!equalTypes (reference_counters, counters))
			if (!containTypes (reference_counters, counters))
			{
				if (common::DEBUG())
				{
					cout << "FoldingWriter::checkSamples divergence between reference counters on timestamp " << Samples[reference]->getTime() << " and " << Samples[u]->getTime() << endl;
				}
				return false;
			}
		}
	}

	return true;
}

void FoldingWriter::Write (ofstream &ofile, const string & RegionName,
	unsigned ptask, unsigned task, unsigned thread,
	unsigned long long start, unsigned long long duration,
	const vector<Sample*> & Samples,
	const vector<DataObject*> &DataObjects,
	const set<unsigned> &livingDataObjects)
{
	/* At least, have a sample at the begin & end, and someone else */
	if (Samples.size() < 2)
	{
		if (common::DEBUG())
		{
			cout << "FoldingWriter::Write Region (" << RegionName << ")" <<
			  " Exiting! Less than 2 samples" << endl;
		}
		return;
	}

	if (!checkSamples (Samples))
	{
		if (common::DEBUG())
		{
			cout << "FoldingWriter::Write Region (" << RegionName << ")" <<
			  " Exiting due to checkSamples" << endl;
		}
		return;
	}

	/* Write the total time spent in this region */

	/* Calculate totals */
	map<string, unsigned long long> totals;
	unsigned reference = getReferenceSample (Samples);
	set<string> Counters = Samples[reference]->getCounters();
	for (const auto & c : Counters)
		totals[c] = 0; // has to accumulate

	/* Generate header for this instance */
	ofile << "I " << ptask+1 << " " << task+1 << " " << thread+1 
	  << " " << RegionName << " " << start << " " << duration;
	for (unsigned u = 0; u < Samples.size(); u++)
		for (const auto & c : Counters)
			if (Samples[u]->hasCounter(c))
				totals[c] = totals[c] + Samples[u]->getCounterValue (c);

	if (common::DEBUG())
	{
		cout << "FoldingWriter::Write Totals at for duration " << duration;
		for (const auto & t : totals)
			cout << " [" << t.first << " " << t.second << "]"; 
		cout << endl;
	}

	ofile << " " << totals.size();
	map<string, unsigned long long>::iterator it_totals;
	for (it_totals = totals.begin(); it_totals != totals.end(); ++it_totals)
		ofile << " " << (*it_totals).first << " " << (*it_totals).second;
	ofile << endl;

	/* End of header */

	/* Dump living and referenced data objects */
	/* 1 search for living data objects that have been referenced by the
	   samples of this instance */
	set<unsigned> livingandreferencedDatAObjects;
	for (const auto & s : Samples)
		if (s->hasAddressReference())
			for (auto DO : livingDataObjects)
			{
				assert (DO < DataObjects.size());
				if (DataObjects[DO]->addressInVariable (s->getAddressReference()))
				{
					livingandreferencedDatAObjects.insert (DO);
					break;
				}
			}
	/* 2 Actual dump */
	ofile << "DO " << livingandreferencedDatAObjects.size();
	for (auto DO : livingandreferencedDatAObjects)
		ofile << " " << DO;
	ofile << endl;
	/* End of dump living and referenced data objects */

	/* Do not emit samples if we only have a sample at the end */
	if (Samples[0]->getTime() == start + duration)
		return;

	/* Prepare partial accum hash */
	map<string, unsigned long long> partials;
	for (const auto & c : Counters)
		partials[c] = 0;

	/* Dump all the samples for this instance except the last one, which can be extrapolated
       from the instance header data */
	for (unsigned u = 0; u < Samples.size() - 1; ++u)
	{
		ofile << "S " << Samples[u]->getTime() << " "
		  << Samples[u]->getiTime() << " ";

		unsigned ncounters = 0;
		for (const auto & c : Counters)
			if (Samples[u]->hasCounter(c))
				ncounters++;
		ofile << ncounters;

		for (const auto & c : Counters)
			if (Samples[u]->hasCounter(c))
			{
				unsigned long long tmp;
				tmp = partials[c] + Samples[u]->getCounterValue (c);
				ofile << " " << c << " " << tmp;
				partials[c] = tmp;
			}

		if (common::DEBUG())
		{
			cout << "FoldingWriter::Write partial at sample time " <<
			  Samples[u]->getTime();
			for (const auto & p : partials)
				cout << " [" << p.first << " " << p.second << "]"; 
			cout << endl;
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

		/* Dump address references */
		if (Samples[u]->hasAddressReference())
		{
			string rt;
			if (Samples[u]->getReferenceType() == LOAD)
				rt = "LD";
			else if (Samples[u]->getReferenceType() == STORE)
				rt = "ST";

			if (Samples[u]->hasAddressReferenceInfo())
			{
				ofile << " 1 " << rt << " "
				  << Samples[u]->getAddressReference() << " 1 "
				  << Samples[u]->getAddressReference_Mem_Level() << " "
				  << Samples[u]->getAddressReference_TLB_Level() << " "
				  << Samples[u]->getAddressReference_Cycles_Cost();
			}
			else
			{
				ofile << " 1 " << rt << " "
				  << Samples[u]->getAddressReference() << " 0 ";
			}
		}
		else
			ofile << " 0";

		ofile << endl;
	}
}
