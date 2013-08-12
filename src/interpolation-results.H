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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/callstackanalysis.C $
 | 
 | @last_commit: $Date: 2013-05-24 16:08:28 +0200 (dv, 24 mai 2013) $
 | @version:     $Revision: 1764 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef INTERPOLATION_RESULTS_H_INCLUDED
#define INTERPOLATION_RESULTS_H_INCLUDED

#include <string>
#include <vector>

class InterpolationResults
{
	private:
	const unsigned count;

	bool slope_calculated;
	double slope_factor;
	double avgCounterValue;
	double *interpolation;
	double *slope;
	string interpolationDetails;
	vector<double> Breakpoints;

	public:
	InterpolationResults (unsigned count);
	~InterpolationResults (void);

	unsigned getCount (void) const
	  { return count; }
	double *getInterpolationResultsPtr (void)
	  { return interpolation; }
	double *getSlopeResultsPtr (void)
	  { return slope; }
	void calculateSlope (double factor);
	bool isSlopeCalculated (void) const
	  { return slope_calculated; }
	double factorSlope (void) const
	  { return slope_calculated?slope_factor:0; }
	void setAvgCounterValue (double d)
	  { avgCounterValue = d; }
	double getAvgCounterValue (void) const
	  { return avgCounterValue; }
	void setInterpolationDetails (string s)
	  { interpolationDetails = s; }
	string getInterpolationDetails (void) const
	  { return interpolationDetails; }
	vector<double> getBreakpoints (void)
	  { return Breakpoints; }
	void setBreakpoints (vector<double> &breakpoints)
	  { Breakpoints = breakpoints; }

	double getSlopeAt (double pos) const;
	double getSlopeAt (unsigned pos) const;
};

#endif /* INTERPOLATION_RESULTS_H_INCLUDED */
