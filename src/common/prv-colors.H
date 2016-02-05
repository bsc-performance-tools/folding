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

#ifndef PRV_COLORS_H_INCLUDED
#define PRV_COLORS_H_INCLUDED

#include <string>
#include <vector>

using namespace std;

typedef struct RGBcolor_st
{
	unsigned char R;
	unsigned char G;
	unsigned char B;

	bool operator==( const RGBcolor_st& b ) const
	{ return R == b.R && G == b.G && B == b.B; }
	
	bool operator!=( const RGBcolor_st& b ) const
	{ return !( R == b.R && G == b.G && B == b.B ); }

} RGBcolor_t;

class PRVcolors
{
	private:
	vector<RGBcolor_st> colors;

	public:
	PRVcolors();
	RGBcolor_t getRGB (unsigned long long);
	string getString (unsigned long long);
};

#endif /* PRV_COLORS_H_INCLUDED */
