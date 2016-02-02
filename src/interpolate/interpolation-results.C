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

#include "common.H"
#include "interpolation-results.H"

#include <assert.h>

InterpolationResults::InterpolationResults (unsigned count) : count(count)
{
	slope_calculated = false;
	interpolation = new double[count];
	slope = new double[count];

	for (unsigned u = 0; u < count; u++)
		interpolation[u] = slope[u] = 0;
}

InterpolationResults::~InterpolationResults (void)
{
	delete [] slope;
	delete [] interpolation;
}

void InterpolationResults::calculateSlope (double factor)
{
	double d_last = interpolation[0];
	double d_inv_count = (1.0 / (double) count);
	slope[0] = 0;

	for (unsigned j = 1; j < count; j++)
		if (d_last < interpolation[j])
		{
			slope[j] = factor * ((interpolation[j]-d_last) / d_inv_count);
			d_last = interpolation[j];
		}
		else
			slope[j] = 0;

	slope_factor = factor;
	slope_calculated = true;
}

double InterpolationResults::getSlopeAt (unsigned pos) const
{
	assert (pos < count);
	return slope[pos];
}

double InterpolationResults::getSlopeAt (double pos) const
{
	assert (pos >= 0 && pos <= 1.0f);
	unsigned index = ((unsigned) (pos * count));
	return getSlopeAt (index);
}

double InterpolationResults::getInterpolationAt (unsigned pos) const
{
	assert (pos < count);
	return interpolation[pos];
}

double InterpolationResults::getInterpolationAt (double pos) const
{
	assert (pos >= 0 && pos <= 1.0f);
	unsigned index = ((unsigned) (pos * count));
	return getInterpolationAt (index);
}

