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

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>

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

			string Y1Limit = ((*it)->metric == "LINE" || (*it)->metric == "LINEID")?"*":"1";

			if ((*it)->interpolated)
			{
				gnuplot_out
			    << "set x2range [0:1];" << endl
			    << "set y2range [0:*];" << endl
			    << "set ytics nomirror;" << endl
			    << "set y2tics nomirror;" << endl
			    << "set x2tics nomirror;" << endl
				  << "set y2label '" << counter << " rate (in Mevents/s)';" << endl;

				gnuplot_out
				  << "set xtics ( 0.0 ";
				for (int i = 1; i <= 5; i++)
				{
					double duration = (*it)->mean_duration / 1000000;
					gnuplot_out << ", " << fixed << setprecision(2) << i*duration/5;
				}

				gnuplot_out << ");" << endl;
				gnuplot_out << endl;
				for (unsigned i = 0; i < (*it)->qualities.size(); i++)
					gnuplot_out << "set label \"Q" << i << " = " << fixed << setprecision(2) << (*it)->qualities[i] << "\" at graph 0.01," << 0.95f - i*0.05 << endl;
				gnuplot_out << endl;
			}

			gnuplot_out
				<< "set xrange [0:" << fixed << setprecision(2) << (*it)->mean_duration / 1000000 << "];" << endl
			  << "set yrange [0:" << Y1Limit << "];" << endl
			  << "set ytics nomirror;" << endl
			  << "set key bottom right invert" << endl
				<< "set title \"" << (*it)->title << "\\n" << 
			     "Duration = " << fixed << setprecision(2) << (*it)->mean_duration / 1000000 << " ms" << 
				   " Counter = " << fixed << setprecision(2) << (*it)->mean_counter/1000 <<  " Kevents" << 
				   "\"" << endl

				<< "set ylabel 'Normalized " << counter << "';" << endl
				<< "set xlabel 'Normalized time';" << endl
				<< "plot '" << file << ".points' using 2:((strcol(1) eq '" << counter << "') ? $3 : 1/0) title 'Samples (" << (*it)->inpoints << ")' axes x2y1 w points";

			if ((*it)->interpolated)
			{
				gnuplot_out
				  << ",\\" << endl
				  << "     '" << file << ".interpolation' using 2:((strcol(1) eq '" << counter << "') ? $3 : 1/0) title 'Curve fitting' axes x2y1 w lines lw 2,\\" << endl
			 	  << "     '" << file << ".slope' using 2:((strcol(1) eq '" << counter << "') ? $3 : 1/0) title 'Counter rate' axes x2y2 w lines lw 2,\\" << endl
			 	  << "     '" << file << ".segmentedslope' using 2:((strcol(1) eq '" << counter << "' && $4 == 0.0006) ? $3 : 1/0) title 'Segment 0.0006' axes x2y1 w points ps 1.5 pt 7 lt rgb '#000000';" << endl;
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

	gnuplot_out
	  << "#set term postscript eps solid enhanced color font \",18\"" << endl
	  << "#set term png" << endl
	  << "set key bottom right invert" << endl;

	for (list<GNUPLOTinfo*>::iterator it = info.begin(); it != info.end() ; it++)
	{
		if ((*it)->done)
		{
			string Y1Limit = ((*it)->metric == "LINE" || (*it)->metric == "LINEID")?"*":"1";

			if ((*it)->interpolated)
			{
	  		gnuplot_out
					<< "set xrange [0:" << fixed << setprecision(2) << (*it)->mean_duration / 1000000 << "];" << endl
				  << "set yrange [0:" << Y1Limit << "];" << endl
				  << "set x2range [0:1];" << endl
				  << "set y2range [0:*];" << endl
				  << "set ytics nomirror;" << endl
				  << "set y2tics nomirror;" << endl
				  << "set x2tics;" << endl;

				gnuplot_out
				  << "set xtics ( 0.0 ";
				for (int i = 1; i <= 5; i++)
				{
					double duration = (*it)->mean_duration / 1000000;
					gnuplot_out << ", " << fixed << setprecision(2) << i*duration/5;
				}

				gnuplot_out << ");" << endl;
					gnuplot_out << endl;
					for (unsigned i = 0; i < (*it)->qualities.size(); i++)
						gnuplot_out << "set label \"Q" << i << " = " << fixed << setprecision(2) << (*it)->qualities[i] << "\" at graph 0.01," << 0.95f - i*0.05 << endl;
					gnuplot_out << endl;
			}
			else
			{
	  		gnuplot_out
					<< "set xrange [0:" << fixed << setprecision(2) << (*it)->mean_duration / 1000000 << "];" << endl
				  << "set yrange [0:" << Y1Limit << "];" << endl
				  << "set x2range [0:1];" << endl
				  << "set y2range [0:1];" << endl
				  << "set ytics mirror;" << endl;
				gnuplot_out
				  << "set xtics ( 0.0 ";
				for (int i = 1; i <= 5; i++)
				{
					double duration = (*it)->mean_duration / 1000000;
					gnuplot_out << ", " << fixed << setprecision(2) << i*duration/5;
				}
				gnuplot_out << ");" << endl;
			}

			string file = (*it)->fileprefix;
			string counter = (*it)->metric;

#warning "TODO set output .fitxer.eps / .fitxer.png"

			gnuplot_out
				<< "set title \"" << (*it)->title << "\\n" << 
			     "Duration = " << fixed << setprecision(2) << (*it)->mean_duration / 1000000 << " ms" << 
				   " Counter = " << fixed << setprecision(2) << (*it)->mean_counter/1000 <<  " Kevents" << 
				   "\"" << endl
				<< "#set title \"" << (*it)->title << "\\n" << 
			     "Duration = " << fixed << setprecision(2) << (*it)->mean_duration / 1000000 << " ms" << 
				   " Counter = " << fixed << setprecision(2) << (*it)->mean_counter/1000 <<  " Kevents" << 
				   "\" font \",24\"" << endl
				<< "set ylabel 'Normalized " << counter << "';" << endl
				<< "set y2label '" << counter << " rate (in Mevents/s)';" << endl
				<< "set xlabel 'Normalized time';" << endl
				<< "plot '" << file << ".points' using 2:((strcol(1) eq '" << counter << "') ? $3 : 1/0) title 'Samples (" << (*it)->inpoints << ")' axes x2y1 w points";

			if ((*it)->interpolated)
			{
				gnuplot_out
				  << ",\\" << endl
				  << "     '" << file << ".interpolation' using 2:((strcol(1) eq '" << counter << "') ? $3 : 1/0) title 'Curve fitting' axes x2y1 w lines lw 2,\\" << endl
			 	  << "     '" << file << ".slope' using 2:((strcol(1) eq '" << counter << "') ? $3 : 1/0) title 'Counter rate' axes x2y2 w lines lw 2, \\" << endl
			 	  << "     '" << file << ".segmentedslope' using 2:((strcol(1) eq '" << counter << "' && $4 == 0.0006) ? $3 : 1/0) title 'BRK 0.0006' axes x2y1 w points ps 1.5 pt 7 lt rgb '#000000';" << endl;
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
	string GNUPLOTfile = file+"."+common::removeSpaces (regionName)+".slopes.gnuplot";

	ofstream gnuplot_out (GNUPLOTfile.c_str());
	if (!gnuplot_out.is_open())
	{
		cerr << "Cannot create " << GNUPLOTfile << " file " << endl;
		exit (-1);
	}

	gnuplot_out
	  << "set key top right invert" << endl
	  << "set xrange [0:1];" << endl
	  << "set yrange [0:*];" << endl
	  << "set ytics mirror;" << endl
	  << "set xtics mirror;" << endl
		<< "set ylabel 'Performance metric rate';" << endl
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
				gnuplot_out << "\\" << endl << "     '" << file << ".slope' using 2:((strcol(1) eq '" << counter << "') ? $3 : 1/0) title '" << counter << "' w lines lw 2";
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

