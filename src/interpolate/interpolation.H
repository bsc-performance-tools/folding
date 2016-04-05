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
#ifndef INTERPOLATION_H_INCLUDED
#define INTERPOLATION_H_INCLUDED

#include "instance-group.H"
#include "interpolation-results.H"

enum { SUCCESS, FAILED };

class Interpolation
{
	protected:
	const unsigned steps;
	const bool prefilter;

	/* This is to invoke the interpolation */
	InterpolationResults * interpolate_kernel (vector<Sample*> vs,
	  const string & counter, const string & region, unsigned group);

	/* Actual interpolation :: need to be redefined in the child class */ 
	virtual unsigned do_interpolate (unsigned inpoints, double *ix,
	  double *iy, InterpolationResults *ir, const string & counter,
	  const string & region, unsigned group) = 0;

	public:

	/* Constructor, steps refers to the number of output points and prefilter
	   whether we want to do some prefiltering */
	Interpolation (unsigned steps, bool prefilter);

	/* Applies the actual interpolation */
	void interpolate (InstanceGroup *ig, const set<string> & counters,
	  const string & TimeUnit);

	/* Applies interpolation for prefiltering */
	virtual void pre_interpolate (double sigmaTimes, InstanceGroup *ig,
		const set<string> & counters);

	/* Return whether the user wants to prefilter */
	bool preFilter (void) const
	  { return prefilter; }

	/* Return the number of output points */
	unsigned getSteps (void) const
	  { return steps; }

	virtual string details (void) const = 0;
};

#endif
