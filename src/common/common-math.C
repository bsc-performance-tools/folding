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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/common/common.H $
 | 
 | @last_commit: $Date: 2014-01-10 18:15:20 +0100 (vie, 10 ene 2014) $
 | @version:     $Revision: 2398 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "common.H"
#include "common-math.H"
#include <assert.h>

void CommonMath::LinearRegression (const vector <pair<double, double> > &v, 
	double &slope, double &intercept, double &correlationcoefficient)
{
	assert (v.size() > 0);

	slope = intercept = correlationcoefficient = 0.;

	/* Details from http://sciencefair.math.iit.edu/analysis/linereg/hand */
	double xsum = 0., ysum = 0., xysum = 0. , xxsum = 0., yysum = 0.;
	double N = v.size();
	for (auto const value : v)
	{
		xsum += value.first;
		ysum += value.second;
		xysum += value.first*value.second;
		xxsum += value.first*value.first;
		yysum += value.second*value.second;
	}

	slope     = ((N*xysum) - (xsum*ysum)) / ((N*xxsum) - (xsum*xsum));
	intercept = ((xxsum*ysum) - (xsum*xysum)) / ((N*xxsum) - (xsum*xsum));
	correlationcoefficient
	          = (N*xysum-xsum*ysum)*(N*xysum-xsum*ysum)/
	            ( (N*xxsum-xsum*xsum)*(N*yysum-ysum*ysum) );
}

