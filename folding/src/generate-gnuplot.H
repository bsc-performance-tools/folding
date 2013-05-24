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

#ifndef GENERATE_GNUPLOT_H
#define GENERATE_GNUPLOT_H

#include <list>
#include <string>
#include "UIParaverTraceConfig.h"
#include "point.H"

using namespace std;
using namespace libparaver;

class GNUPLOTinfo
{
	public:
	int inpoints;
	bool done;
	bool interpolated;
	double mean_duration;
	double mean_counter;
	string title;
	string fileprefix;
	string metric;
	string nameregion;
	vector <double> qualities;
	vector <double> breakpoints;
};

void createMultipleGNUPLOT (list<GNUPLOTinfo*> &info);

void createSingleGNUPLOT (string file, list<GNUPLOTinfo*> &info);

void createMultiSlopeGNUPLOT (string file, string regionName, list<GNUPLOTinfo*> &info, vector <string> &wantedCounters);

void createAccumulatedCounterGNUPLOT (string file, string regionName, vector<Point> &points, vector <string> &wantedCounters);

#endif
