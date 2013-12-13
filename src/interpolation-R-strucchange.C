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
 | @file: $HeadURL$
 | 
 | @last_commit: $Date$
 | @version:     $Revision$
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id$";

#include "common.H"

#include "interpolation-R-strucchange.H"
#include "execute-R.H"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <unistd.h>


InterpolationRstrucchange::InterpolationRstrucchange (unsigned steps, double h) 
	: Interpolation(steps, false), h(h)
{
#if defined(HAVE_R_DOPARALLEL)
	bool b;
	nprocs = common::getNumCores(b);
	if (!b)
	{
		nprocs = common::getNumProcessors(b);
		if (!b)
			nprocs = 1;
	}
#else
	nprocs = 1;
#endif
}

string InterpolationRstrucchange::details (void) const
{
	stringstream ss;
	ss << setprecision(1) << scientific << h;
	return string ("R+strucchange (h=") + ss.str() + ",np=" + common::convertInt (nprocs) + ")";
}


unsigned InterpolationRstrucchange::do_interpolate (unsigned inpoints,
	double *ix, double *iy, InterpolationResults *ir, string counter,
	string region, unsigned group)
{
	vector<double> breakpoints, slopes;
	ofstream file;
	string f, fo;

	if (getenv("TMPDIR") == NULL)
		f = "/tmp/folding.R.";
	else
		f = string (getenv("TMPDIR")) + "/folding.R.";
	f = f + common::removeUnwantedChars(region) + "." + common::convertInt (group)
	  + "." + common::convertInt (getpid());
	fo = f + ".output";

	file.open (f.c_str());
	if (file.is_open())
	{
		file << setprecision (6);
		file << "time;value" << endl;
		for (unsigned p = 0; p < inpoints; p++)
			file << ix[p] << ";" << iy[p] << endl;
		file.close();

#define R_BREAKPOINTS_FILE "breakpoints.R"

		string Rcommands = string("\n")+
		  "source (\"" + getenv ("FOLDING_HOME") +"/etc/" + R_BREAKPOINTS_FILE + "\")\n\n" +
		  "sink (\"" + fo + "\")\n" +
		  "DEBUG <- FALSE\n" +
		  "H <- " + common::convertDouble (h, 6) + "\n" +
		  "GROUP <- " + common::convertInt (group) + "\n" +
		  "COUNTER <- '" + counter + "'\n" +
		  "FILE <- \"" + f + "\"\n" +
		  "NSTEPS <- 0\n" +
		  "MAX_ERROR <- 0.002\n" +
		  "NCORES <- " + common::convertInt (nprocs) + "\n" +
		  "main (FILE, H, NSTEPS, MAX_ERROR, COUNTER, GROUP, NCORES)\n";

#undef R_BREAKPOINTS_FILE

		breakpoints.clear();
		slopes.clear();

		R::launch (f+".script", Rcommands,
		 "The strucchange has failed. This is likely to happen when:\n"
         " - H is too low (try increasing H to fix)"
		 "\n"
		 "If none of the above can be applied, look at TMPDIR for the execution scripts and outputs\n");

		ifstream resfile;
		resfile.open (fo.c_str());
		if (resfile.is_open())
		{
			string s;
			bool found = false;
			found = false;

			while (!found)
				if (getline(resfile, s))
					found = s == "[1] \"FINAL-RESULTS\"";
				else
					break;

			if (found)
			{
				string unused;
				unsigned number_entries;

				resfile >> unused;
				resfile >> number_entries;

				for (unsigned u = 0; u < number_entries; u++)
				{
					double db;
					resfile >> unused;
					resfile >> db;
					breakpoints.push_back (db);
				}
				for (unsigned u = 0; u < number_entries-1; u++)
				{
					double db;
					resfile >> unused;
					resfile >> db;
					slopes.push_back (db > 0 ? db : 0.0f);
				}
			}

			/* Write results to output buffer */
			unsigned o_count = ir->getCount();
			double *o_ptr = ir->getInterpolationResultsPtr();

			double partial = 0.0f;
			for (unsigned s = 0; s < o_count; s++)
			{
				double t = ((double) s) / ((double) o_count);
				for (unsigned i = 0; i < breakpoints.size()-1; i++)
					if (t >= breakpoints[i] && t < breakpoints[i+1])
					{
						partial += slopes[i] * (1.0f / (double) o_count);
						o_ptr[s] = partial;
					}
			}

			/* Write breakpoints to the results */
			ir->setBreakpoints (breakpoints);
		}
	}
	else
	{
		cerr << "Error! Unable to create temporary file '" << f
		  << "' to execute R command" << endl;
		exit (-1);
	}

	if (common::DEBUG())
	{
		for (unsigned u = 0; u < breakpoints.size()-1; u++)
		{
			cout << "R returned phase " << u+1 << " between [ " 
			  << breakpoints[u] << "," << breakpoints[u+1]
			  << " ] for a slope " << slopes[u] << endl;
		}
	}

	if (unlink (f.c_str()))
		cerr << "Warning! Could not remove temporal file '" << f
		  << "'" << endl;

	return SUCCESS;
}


