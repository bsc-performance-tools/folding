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
#ifndef FOLDING_WRITER_H_INCLUDED
#define FOLDING_WRITER_H_INCLUDED

#include <string>
#include <set>
#include <fstream>
#include <vector>

#include "sample.H"
#include "data-object.H"

using namespace std;

class FoldingWriter
{
	private:
	static bool equalTypes (const set<string> &m1, const set<string> m2);
	static bool containTypes (const set<string> &m1, const set<string> m2);
	static bool checkSamples (const vector<Sample*> &Samples);
	static unsigned getReferenceSample (const vector<Sample*> &Samples);
	static set<string> minimumCounterSet (const vector<Sample*> &Samples);

	public:
	static void Write (ofstream & ofile, const string & RegionName,
	  unsigned ptask, unsigned task, unsigned thread,
	  unsigned long long start, unsigned long long duration,
	  const vector<Sample*> & Samples,
	  const vector<DataObject*> &DataObjects,
	  const set<unsigned> & livingDataObjects );
};

#endif /* FOLDING_WRITER_H_INCLUDED */

