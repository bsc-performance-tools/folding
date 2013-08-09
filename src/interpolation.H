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

#ifndef INTERPOLATION_H_INCLUDED
#define INTERPOLATION_H_INCLUDED

#include "instance-group.H"
#include "interpolation-results.H"

enum { SUCCESS, FAILED };

class Interpolation
{
	protected:
	unsigned steps;
	bool prefilter;

	InterpolationResults * interpolate_kernel (vector<Sample*> vs,
	  string counter, string region, unsigned group);

	virtual unsigned do_interpolate (unsigned inpoints, double *ix,
	  double *iy, InterpolationResults *ir, string counter,
	  string region, unsigned group) = 0;

	public:
	Interpolation (unsigned steps, bool prefilter);
	void interpolate (InstanceGroup *ig, set<string> &counters);
	virtual void pre_interpolate (double sigmaTimes, InstanceGroup *ig, set<string> &counters);
	bool preFilter (void) const
	  { return prefilter; }
	unsigned getSteps (void) const
	  { return steps; }

	virtual string details (void) const = 0;
};

#endif
