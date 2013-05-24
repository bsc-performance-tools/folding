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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/folding/src/interpolate.C $
 | 
 | @last_commit: $Date: 2013-01-04 16:23:26 +0100 (dv, 04 gen 2013) $
 | @version:     $Revision: 1449 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id: interpolate.C 1449 2013-01-04 15:23:26Z harald $";

#include "execute-breakpoints-R.H"
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>

using namespace std;

void Rbreakpoints::breakpoints (string file, string counter, unsigned size, double h,
	vector<double> &slopes, vector<double> &breakpoints, vector<unsigned> &excludedinstances)
{
	stringstream ss, nsamples;
	ss << getpid(); 
	nsamples << size;
	string output_file = string ("/tmp/R.") + ss.str() + ".output";
	stringstream size_s, h_s;
	size_s << size;
	h_s << h;
	char *folding;

	if ((folding = getenv ("FOLDING_HOME")) == NULL)
	{
		cerr << "Error! Unset FOLDING_HOME! Set it before executing this application." << endl;
		exit (0);
	}

	string Rcommands = string("\n")+
		"source (\"" + folding +"/etc/breakpoints.R\")\n\n" +
		"sink (\"" + output_file + "\")\n" +
		"file <- \"" + file + "\"\n" +
		"COUNTER <- \"" + counter + "\"\n" +
		"H <- 25\n" +
		"NSTEPS <- 1\n" +
		"MAX_ERROR <- 0.001\n"+
		"NSAMPLES <- "+nsamples.str()+"\n"+
		"DEBUG <- TRUE\n"+
		"\n"+
		"main (COUNTER, file, H, NSTEPS, MAX_ERROR, NSAMPLES)\n";

	unlink (output_file.c_str());
	R::launch (Rcommands);

	breakpoints.clear();
	slopes.clear();

	ifstream generatedfile;
	generatedfile.open (output_file.c_str());
	if (generatedfile.is_open())
	{
		bool found = false;
		string s;
		while (!found)
			if (getline(generatedfile, s))
				found = s == "[1] \"REMOVE-INSTANCES\"";
			else
				break;

		if (found)
		{
			string unused;
			unsigned number_entries;

			generatedfile >> unused;
			generatedfile >> number_entries;

			for (unsigned u = 0; u < number_entries; u++)
			{
				unsigned ub;
				generatedfile >> unused;
				generatedfile >> ub;
				excludedinstances.push_back (ub);
			}
		}

		found = false;
		while (!found)
			if (getline(generatedfile, s))
				found = s == "[1] \"FINAL-RESULTS\"";
			else
				break;

		if (found)
		{
			string unused;
			unsigned number_entries;

			generatedfile >> unused;
			generatedfile >> number_entries;

			for (unsigned u = 0; u < number_entries; u++)
			{
				double db;
				generatedfile >> unused;
				generatedfile >> db;
				breakpoints.push_back (db);
			}
			for (unsigned u = 0; u < number_entries-1; u++)
			{
				double db;
				generatedfile >> unused;
				generatedfile >> db;
				slopes.push_back (db > 0 ? db : 0.0f);
			}
		}
	}

#if 0
	cout << " RESULTS_BPT = " << endl;
	for (unsigned u = 0; u < breakpoints.size(); u++)
		cout << "bpt[" << u << "]=" << breakpoints[u] << endl;
	for (unsigned u = 0; u < slopes.size(); u++)
		cout << "slope[" << u << "]=" << slopes[u] << endl;
	for (unsigned u = 0; u < excludedinstances.size(); u++)
		cout << "ei[" << u << "]=" << excludedinstances[u] << endl;
#endif
}


