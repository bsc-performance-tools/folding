/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  MPItrace                                 *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *                                                             ___           *
 *   +---------+     http:// www.cepba.upc.edu/tools_i.htm    /  __          *
 *   |    o//o |     http:// www.bsc.es                      /  /  _____     *
 *   |   o//o  |                                            /  /  /     \    *
 *   |  o//o   |     E-mail: cepbatools@cepba.upc.edu      (  (  ( B S C )   *
 *   | o//o    |     Phone:          +34-93-401 71 78       \  \  \_____/    *
 *   +---------+     Fax:            +34-93-401 25 77        \  \__          *
 *    C E P B A                                               \___           *
 *                                                                           *
 * This software is subject to the terms of the CEPBA/BSC license agreement. *
 *      You must accept the terms of this license to use this software.      *
 *                                 ---------                                 *
 *                European Center for Parallelism of Barcelona               *
 *                      Barcelona Supercomputing Center                      *
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

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "generate-gnuplot.H"

void createMultipleGNUPLOT (list<GNUPLOTinfo*> &info)
{
	for (list<GNUPLOTinfo*>::iterator it = info.begin(); it != info.end() ; it++)
	{
		if ((*it)->done)
		{
			string file = (*it)->fileprefix;
			string counter = (*it)->metric;
			string GNUPLOTfile = file + "." + counter + ".gnuplot";

			ofstream gnuplot_out (GNUPLOTfile.c_str());
			if (!gnuplot_out.is_open())
			{
				cerr << "Cannot create " << GNUPLOTfile << " file " << endl;
				exit (-1);
			}

			/* Set precision for floating point values */
			gnuplot_out.precision (3);

			string Y1Limit = ((*it)->metric == "LINE" || (*it)->metric == "LINEID")?"*":"1";

			if ((*it)->interpolated)
			{
				gnuplot_out
			    << "set x2range [0:1];" << endl
			    << "set y2range [0:*];" << endl
			    << "set y2tics;" << endl
			    << "set x2tics;" << endl
				  << "set y2label 'Slope of " << counter << "';" << endl;
			}

			gnuplot_out
				<< "set xrange [0:1];" << endl
			  << "set yrange [0:" << Y1Limit << "];" << endl
			  << "set ytics nomirror;" << endl
			  << "set xtics nomirror;" << endl
			  << "set key bottom right" << endl
				<< "set title '" << (*it)->title << " - with " << (*it)->error << " error';" << endl
				<< "set ylabel '" << counter << "';" << endl
				<< "set xlabel 'Normalized time';" << endl
				<< "plot '" << file << "." << counter << ".points' using 2:3 title 'Samples'";

			if ((*it)->interpolated)
			{
				gnuplot_out
				  << ",\\" << endl
				  << "     '" << file << "." << counter << ".interpolation' using 2:3 title 'Curve fitting' w lines lw 2,\\" << endl
			 	  << "     '" << file << "." << counter << ".slope' using 2:3 title 'Curve fitting slope' axes x2y2 w lines lw 2;" << endl;
			}
			else
				gnuplot_out << ";" << endl;

			gnuplot_out.close();
		}
	}
}

void createSingleGNUPLOT (string file, list<GNUPLOTinfo*> &info)
{
	string GNUPLOTfile = file+".gnuplot";

	ofstream gnuplot_out (GNUPLOTfile.c_str());
	if (!gnuplot_out.is_open())
	{
		cerr << "Cannot create " << GNUPLOTfile << " file " << endl;
		exit (-1);
	}

	/* Set precision for floating point values */
	gnuplot_out.precision (3);

	gnuplot_out
	  << "set key bottom right;" << endl;

	for (list<GNUPLOTinfo*>::iterator it = info.begin(); it != info.end() ; it++)
	{
		if ((*it)->done)
		{
			string Y1Limit = ((*it)->metric == "LINE" || (*it)->metric == "LINEID")?"*":"1";

			if ((*it)->interpolated)
			{
	  		gnuplot_out
				  << "set xrange [0:1];" << endl
				  << "set yrange [0:" << Y1Limit << "];" << endl
				  << "set x2range [0:1];" << endl
				  << "set y2range [0:*];" << endl
				  << "set y2tics;" << endl
				  << "set x2tics;" << endl;
			}
			else
			{
	  		gnuplot_out
				  << "set xrange [0:1];" << endl
				  << "set yrange [0:" << Y1Limit << "];" << endl
				  << "set x2range [0:1];" << endl
				  << "set y2range [0:1];" << endl
				  << "set ytics mirror;" << endl
				  << "set xtics mirror;" << endl;
			}

			string file = (*it)->fileprefix;
			string counter = (*it)->metric;

			gnuplot_out
				<< "set title '" << (*it)->title << " - with " << (*it)->error << " error';" << endl
				<< "set ylabel '" << counter << "';" << endl
				<< "set y2label 'Slope of " << counter << "';" << endl
				<< "set xlabel 'Normalized time';" << endl
				<< "plot '" << file << "." << counter << ".points' using 2:3 title 'Samples'";

			if ((*it)->interpolated)
			{
				gnuplot_out
				  << ",\\" << endl
				  << "     '" << file << "." << counter << ".interpolation' using 2:3 title 'Curve fitting' w lines lw 2,\\" << endl
			 	  << "     '" << file << "." << counter << ".slope' using 2:3 title 'Curve fitting slope' axes x2y2 w lines lw 2;" << endl;
			}
			else
				gnuplot_out << ";" << endl;

			gnuplot_out << "pause -1;" << endl << endl;
		}
	}

	gnuplot_out.close();
}

void createMultiSlopeGNUPLOT (string file, string regionName, list<GNUPLOTinfo*> &info, vector <string> &wantedCounters)
{
	string GNUPLOTfile = file+"."+regionName.substr (0, regionName.find_first_of (":[]{}() "))+".slopes.gnuplot";

	ofstream gnuplot_out (GNUPLOTfile.c_str());
	if (!gnuplot_out.is_open())
	{
		cerr << "Cannot create " << GNUPLOTfile << " file " << endl;
		exit (-1);
	}

	gnuplot_out
	  << "set key top right;" << endl
	  << "set xrange [0:1];" << endl
	  << "set yrange [0:*];" << endl
	  << "set ytics mirror;" << endl
	  << "set xtics mirror;" << endl
		<< "set ylabel 'Slope of performance metric';" << endl
		<< "set xlabel 'Normalized time';" << endl;

	bool found = false;
	bool first = true;
	for (list<GNUPLOTinfo*>::iterator it = info.begin(); it != info.end() ; it++)
	{
		string counter = (*it)->metric;
		string file = (*it)->fileprefix;

		if ((*it)->nameregion == regionName)
		{
			if (!found)
			{
				gnuplot_out << "set title '" << (*it)->title << "';" << endl;
  			gnuplot_out << "plot ";
				found = true;
			}
			if ((*it)->interpolated && (*it)->done &&
		    find (wantedCounters.begin(), wantedCounters.end(), counter) != wantedCounters.end())
			{
				if (!first)
					gnuplot_out << ",";
				gnuplot_out << "\\" << endl << "     '" << file << "." << counter << ".slope' using 2:3 title '" << counter << "' w lines lw 2";
				first = false;
			}
		}
	}
	if (found)
		gnuplot_out << ";" << endl;

	gnuplot_out.close();

	if (!found)
		remove (GNUPLOTfile.c_str());
}

