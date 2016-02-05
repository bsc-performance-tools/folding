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

#include "prv-colors.H"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <ext/hash_set>

/* This was given by Paraver developers */

#define MAX_PRV_COLORS 49
#define MAX_COLORS     32000

static RGBcolor_t __PRVcolors[MAX_PRV_COLORS] = {
  { 117, 195, 255 }, //  0 - Idle
  {   0,   0, 255 }, //  1 - Running
  { 255, 255, 255 }, //  2 - Not created
  { 255,   0,   0 }, //  3 - Waiting a message
  { 255,   0, 174 }, //  4 - Blocked
  { 179,   0,   0 }, //  5 - Thread Synchronization
  { 0,   255,   0 }, //  6 - Test/Probe
  { 255, 255,   0 }, //  7 - Scheduled and Fork/Join
  { 235,   0,   0 }, //  8 - Wait/Wait all
  {   0, 162,   0 }, //  9 - Blocked
  { 255,   0, 255 }, // 10 - Immediate Send
  { 100, 100, 177 }, // 11 - Immediate Receive
  { 172, 174,  41 }, // 12 - I/O
  { 255, 144,  26 }, // 13 - Group communication
  {   2, 255, 177 }, // 14 - Tracing Disabled
  { 192, 224,   0 }, // 15 - Overhead
  {  66,  66,  66 }, // 16 - Not used
  { 189, 168, 100 }, // 17 - Not used
  {  95, 200,   0 }, // 18 - Not used
  { 203,  60,  69 }, // 19 - Not used
  {   0, 109, 255 }, // 20 - Not used
  { 200,  61,  68 }, // 21 - Not used
  { 200,  66,   0 }, // 22 - Not used
  {   0,  41,   0 }, // 23 - Not used
  { 139, 121, 177 }, // 24 - Not used
  { 116, 116, 116 }, // 25 - Not used
  { 200,  50,  89 }, // 26 - Not used
  { 255, 171,  98 }, // 27 - Not used
  {   0,  68, 189 }, // 28 - Not used
  {  52,  43,   0 }, // 29 - Not used
  { 255,  46,   0 }, // 30 - Not used
  { 100, 216,  32 }, // 31 - Not used
  {   0,   0, 112 }, // 32 - Not used
  { 105, 105,   0 }, // 33 - Not used
  { 132,  75, 255 }, // 34 - Not used
  { 184, 232,   0 }, // 35 - Not used
  {   0, 109, 112 }, // 36 - Not used
  { 189, 168, 100 }, // 37 - Not used
  { 132,  75,  75 }, // 38 - Not used
  { 255,  75,  75 }, // 39 - Not used
  { 255,  20,   0 }, // 40 - Not used
  {  80,   0,   0 }, // 41 - Not used
  {   0,  66,   0 }, // 42 - Not used
  { 184, 132,   0 }, // 43 - Not used
  { 100,  16,  32 }, // 44 - Not used
  { 146, 255, 255 }, // 45 - Not used
  {   0,  23,  37 }, // 46 - Not used
  { 146,   0, 255 }, // 47 - Not used
  {   0, 138, 119 }  // 48 - Not used
};

struct eqrgb
{
  bool operator()( RGBcolor_st c1, RGBcolor_st c2 ) const
  { return c1 == c2; }
};

struct hashrgb
{
  size_t operator()( RGBcolor_st c ) const
  { return c.R + ( c.B * 256 ) + ( c.G * 65536 ); }
};

// typedef unsigned char ParaverColor;

// enum colorIndex { RED, GREEN, BLUE };

PRVcolors::PRVcolors()
{
	for (auto u = 0; u < MAX_PRV_COLORS; u++)
	{
		RGBcolor_st c;
		c.R = __PRVcolors[u].R;
		c.G = __PRVcolors[u].G;
		c.B = __PRVcolors[u].B;
		colors.push_back(c);
	}

	unsigned iterations = MAX_COLORS / colors.size() / 3;
	unsigned numBaseColors = colors.size();

	__gnu_cxx::hash_set<RGBcolor_st, hashrgb, eqrgb> insertedColors;
	insertedColors.insert (colors.begin(), colors.end());

	unsigned baseColor = 0;
	for( unsigned i = 0; i < iterations; ++i )
	{
		while( baseColor > colors.size() - 1 )
			--baseColor;
		for( unsigned redBaseColor = baseColor; redBaseColor < numBaseColors + baseColor; ++redBaseColor )
		{
			if( redBaseColor > colors.size() - 1 )
				break;
			RGBcolor_st tmp = colors[ redBaseColor ];
			++tmp.R;
			auto result = insertedColors.insert( tmp );
			if( result.second )
				colors.push_back( tmp );
		}
		
		for( unsigned greenBaseColor = baseColor; greenBaseColor < numBaseColors + baseColor; ++greenBaseColor )
		{
			if( greenBaseColor > colors.size() - 1)
				break;
			RGBcolor_st tmp = colors[ greenBaseColor ];
			++tmp.G;
			auto result = insertedColors.insert( tmp );
			if( result.second )
				colors.push_back( tmp );
		}
		
		for( unsigned blueBaseColor = baseColor; blueBaseColor < numBaseColors + baseColor; ++blueBaseColor )
		{
			if( blueBaseColor > colors.size() - 1 )
				break;
			RGBcolor_st tmp = colors[ blueBaseColor ];
			++tmp.B;
			auto result = insertedColors.insert( tmp );
			if( result.second )
				colors.push_back( tmp );
		}
	
		baseColor += numBaseColors;
	}
}

RGBcolor_t PRVcolors::getRGB (unsigned long long v)
{
	unsigned long long index = v % colors.size();
	return colors[index];
}

string PRVcolors::getString (unsigned long long v)
{
	RGBcolor_t c = PRVcolors::getRGB (v);
	stringstream ss;
	ss << uppercase << setfill('0') << setw(2) << hex <<
	  (unsigned)c.R << setw(2) << (unsigned)c.G << setw(2) << (unsigned)c.B;
	return ss.str();
}
