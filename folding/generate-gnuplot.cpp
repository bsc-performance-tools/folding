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

static char rcsid[] = "$Id$";

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "generate-gnuplot.h"

void createMultipleGNUPLOT (unsigned task, unsigned thread, string file,
	string &counterID, int numRegions, string *nameRegion,
	list<string> &GNUPLOTinfo_points, list<bool> &GNUPLOTinfo_interpolated)
{
	list<string>::iterator it_p = GNUPLOTinfo_points.begin();
	list<bool>::iterator it_i = GNUPLOTinfo_interpolated.begin();
	for (int i = 0; i < numRegions; i++, it_p++, it_i++)
	{
		if (*it_i)
		{
			string choppedNameRegion = nameRegion[i].substr (0, nameRegion[i].find(":"));
			string GNUPLOTfile = file+"."+choppedNameRegion+"."+counterID+".gnuplot";

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
			  << "set y2range [0:2];" << endl
			  << "set ytics nomirror;" << endl
			  << "set xtics nomirror;" << endl
			  << "set y2tics;" << endl
			  << "set x2tics;" << endl
			  << "set key bottom right" << endl
				<< "set title ' Task " << task << " Thread " << thread << " -- "<< nameRegion[i] << "' ;" << endl
				<< "set ylabel '" << counterID << "';" << endl
				<< "set y2label 'Slope of " << counterID << "';" << endl
				<< "set xlabel 'Normalized time';" << endl
				<< "plot '" << (*it_p) << "." << counterID << ".points' using 2:3 title 'Samples',\\" << endl
				<< "     '" << (*it_p) << "." << counterID << ".interpolation' using 2:3 title 'Curve fitting' w lines lw 2,\\" << endl
			 	<< "     '" << (*it_p) << "." << counterID << ".slope' using 2:3 title 'Curve fitting slope' axes x2y2 w lines lw 2;" << endl
				<< endl;

			gnuplot_out.close();
		}
	}
}

void createSingleGNUPLOT (unsigned task, unsigned thread, string file,
	string &counterID, int numRegions, string *nameRegion,
	list<string> &GNUPLOTinfo_points, list<bool> &GNUPLOTinfo_interpolated)
{
	string GNUPLOTfile = file+"."+counterID+".gnuplot";

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
	  << "set y2range [0:2];" << endl
	  << "set ytics nomirror;" << endl
	  << "set xtics nomirror;" << endl
	  << "set y2tics;" << endl
	  << "set x2tics;" << endl
	  << "set key bottom right;" << endl;

	list<string>::iterator it_p = GNUPLOTinfo_points.begin();
	list<bool>::iterator it_i = GNUPLOTinfo_interpolated.begin();
	for (int i = 0; i < numRegions; i++, it_p++, it_i++)
	{
		if (*it_i)
			gnuplot_out
				<< "set title ' Task " << task << " Thread " << thread << " -- "<< nameRegion[i] << "' ;" << endl
				<< "set ylabel '" << counterID << "';" << endl
				<< "set y2label 'Slope of " << counterID << "';" << endl
				<< "set xlabel 'Normalized time';" << endl
				<< "plot '" << (*it_p) << "." << counterID << ".points' using 2:3 title 'Samples',\\" << endl
				<< "     '" << (*it_p) << "." << counterID << ".interpolation' using 2:3 title 'Curve fitting' w lines lw 2,\\" << endl
			 	<< "     '" << (*it_p) << "." << counterID << ".slope' using 2:3 title 'Curve fitting slope' axes x2y2 w lines lw 2;" << endl
				<< "pause -1;" << endl
				<< endl;
	}

	gnuplot_out.close();
}

