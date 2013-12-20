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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/read-extracted-data.H $
 | 
 | @last_commit: $Date: 2013-10-25 17:11:45 +0200 (Fri, 25 Oct 2013) $
 | @version:     $Revision: 2250 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef FOLDING_WRITER_H_INCLUDED
#define FOLDING_WRITER_H_INCLUDED

#include <string>
#include <set>
#include <fstream>
#include <vector>

#include "sample.H"

using namespace std;

class FoldingWriter
{
	private:
	static bool equalTypes (set<string> &m1, set<string> m2);
	static bool checkSamples (vector<Sample*> &Samples);

	public:
	static void Write (ofstream & ofile, const string & RegionName,
	  unsigned ptask, unsigned task, unsigned thread,
	  unsigned long long start, unsigned long long duration,
	  vector<Sample*> &Samples);
};

#endif /* FOLDING_WRITER_H_INCLUDED */

