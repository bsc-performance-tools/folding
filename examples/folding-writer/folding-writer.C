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

#include "folding-writer.H"
#include <fstream>

using namespace std;

int main (int argc, char *argv[])
{
	string nameRegion = "FunctionA";
	unsigned long long startRegion = 1000;
	unsigned long long durationRegion = 4500;

	/* NOTE:: Counters are given in deltas from their previous read, not as absolute values */
	map<string, unsigned long long> c1;
	c1["PAPI_TOT_INS"] = 1000;
	c1["PAPI_TOT_CYC"] = 2000;
	map<unsigned, CodeRefTriplet> crt1;
	Sample *s1 = new Sample (2000, 2000-startRegion, c1, crt1);

	map<string, unsigned long long> c2;
	c2["PAPI_TOT_INS"] = 1000;
	c2["PAPI_TOT_CYC"] = 2000;
	map<unsigned, CodeRefTriplet> crt2;
	CodeRefTriplet codeinfo_l0 (1,2,3);
	CodeRefTriplet codeinfo_l1 (3,4,5);
	crt2[0] = codeinfo_l0;
	crt2[1] = codeinfo_l1;
	Sample *s2 = new Sample (4000, 4000-startRegion, c2, crt2);

	/* Last sample typically coincides with end of region -- see durationRegion,
	   Folding::Write won't emit in a S entry */
	map<string, unsigned long long> c3;
	c3["PAPI_TOT_INS"] = 500;
	c3["PAPI_TOT_CYC"] = 1000;
	map<unsigned, CodeRefTriplet> crt3;
	Sample *s3 = new Sample (4500, 4500-startRegion, c3, crt3);

	vector<Sample*> vs;
	vs.push_back (s1);
	vs.push_back (s2);
	vs.push_back (s3);

	ofstream f("output.extract");
	if (f.is_open())
	{
		FoldingWriter::Write (f, nameRegion, 1, 1, 1, startRegion,
		  durationRegion, vs);
		f.close();
	}

	return 0;
}
