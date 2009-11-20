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
#include "generate-gnuplot.h"

void createMultipleGNUPLOT (unsigned task, unsigned thread, string file,
	int numRegions, string *nameRegion, list<GNUPLOTinfo*> &info,
	UIParaverTraceConfig *pcf)
{
	for (list<GNUPLOTinfo*>::iterator it = info.begin(); it != info.end() ; it++)
	{
		if ((*it)->interpolated)
		{
			string file = (*it)->fileprefix;
			string counter = (*it)->counter;
			string GNUPLOTfile = file + "." + counter + ".gnuplot";

			ofstream gnuplot_out (GNUPLOTfile.c_str());
			if (!gnuplot_out.is_open())
			{
				cerr << "Cannot create " << GNUPLOTfile << " file " << endl;
				exit (-1);
			}

			gnuplot_out
				<< "set xrange [0:1];" << endl
			  << "set yrange [0:1];" << endl
			  << "set x2range [0:1];" << endl
			  << "set y2range [0:*];" << endl
			  << "set ytics nomirror;" << endl
			  << "set xtics nomirror;" << endl
			  << "set y2tics;" << endl
			  << "set x2tics;" << endl
			  << "set key bottom right" << endl
				<< "set title '" << (*it)->title << "';" << endl
				<< "set ylabel '" << counter << "';" << endl
				<< "set y2label 'Slope of " << counter << "';" << endl
				<< "set xlabel 'Normalized time';" << endl
				<< "plot '" << file << "." << counter << ".points' using 2:3 title 'Samples',\\" << endl
				<< "     '" << file << "." << counter << ".interpolation' using 2:3 title 'Curve fitting' w lines lw 2,\\" << endl
			 	<< "     '" << file << "." << counter << ".slope' using 2:3 title 'Curve fitting slope' axes x2y2 w lines lw 2;" << endl
				<< endl;

			gnuplot_out.close();
		}
	}
}

void createSingleGNUPLOT (unsigned task, unsigned thread, string file,
	int numRegions, string *nameRegion, list<GNUPLOTinfo*> &info,
	UIParaverTraceConfig *pcf)
{
	string GNUPLOTfile = file+".gnuplot";

	ofstream gnuplot_out (GNUPLOTfile.c_str());
	if (!gnuplot_out.is_open())
	{
		cerr << "Cannot create " << GNUPLOTfile << " file " << endl;
		exit (-1);
	}

	gnuplot_out
		<< "set xrange [0:1];" << endl
	  << "set yrange [0:1];" << endl
	  << "set x2range [0:1];" << endl
	  << "set y2range [0:*];" << endl
	  << "set ytics nomirror;" << endl
	  << "set xtics nomirror;" << endl
	  << "set y2tics;" << endl
	  << "set x2tics;" << endl
	  << "set key bottom right;" << endl;

	for (list<GNUPLOTinfo*>::iterator it = info.begin(); it != info.end() ; it++)
	{
		if ((*it)->interpolated)
		{
			string file = (*it)->fileprefix;
			string counter = (*it)->counter;

			gnuplot_out
				<< "set title '" << (*it)->title << "';" << endl
				<< "set ylabel '" << counter << "';" << endl
				<< "set y2label 'Slope of " << counter << "';" << endl
				<< "set xlabel 'Normalized time';" << endl
				<< "plot '" << file << "." << counter << ".points' using 2:3 title 'Samples',\\" << endl
				<< "     '" << file << "." << counter << ".interpolation' using 2:3 title 'Curve fitting' w lines lw 2,\\" << endl
			 	<< "     '" << file << "." << counter << ".slope' using 2:3 title 'Curve fitting slope' axes x2y2 w lines lw 2;" << endl
				<< "pause -1;" << endl
				<< endl;
		}
	}

	gnuplot_out.close();
}

