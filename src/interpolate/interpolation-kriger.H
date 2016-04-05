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
#ifndef INTERPOLATION_KRIGER_H_INCLUDED
#define INTERPOLATION_KRIGER_H_INCLUDED

#include "interpolation.H"

/* Implementation of the interpolation class using the Kriger interpolation */

class InterpolationKriger : public Interpolation
{
	private:
	const double nuget;

	unsigned do_interpolate (unsigned inpoints,
		double *ix, double *iy, InterpolationResults *ir, const string & counter,
		const string & region, unsigned group);

	double Distance_Point_To_Interpolate (double inpoint_x, double inpoint_y,
	  int outcount, const double *outpoints);

	public:

	/* Constructor for InterpolationKriger. steps refer to the number of output
	   samples. nuget is about the Kriging interpolation (the smaller the value,
	   the more fitted but also more noisy). prefilter = true if you want
	   prefiltering the data using the Kriger interpolation */
	InterpolationKriger (unsigned steps, double nuget, bool prefilter);

	void pre_interpolate (double sigmaTimes, InstanceGroup *ig, 
	  const set<string> & counters);
	string details (void) const;
};

#endif
