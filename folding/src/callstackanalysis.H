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

#ifndef CALLSTACKANALYSIS_H_INCLUDED
#define CALLSTACKANALYSIS_H_INCLUDED

#include <vector>

using namespace std;

class ca_callstacksample
{
	public:
	float Time;
	unsigned Region;
//	vector<unsigned> depth;
	vector<unsigned> caller;
	vector<unsigned> callerline;
};


#include <string>
#include "UIParaverTraceConfig.h"
#include "treenodeholder.H"

#if HAVE_CUBE
#include <cube3/Cube.h>

class TreeNodeHolder;


class ca_callstackanalysis_presence
{
	public:
	unsigned occurrences;
	vector<unsigned> caller;
	vector<unsigned> callerline;
};

class ca_callstackanalysis
{
	public:

	static unsigned do_analysis (string, double, unsigned, string, vector<double> &, 
	  vector<ca_callstacksample> &, libparaver::UIParaverTraceConfig *, cube::Cube*);
	static void do_analysis_presence_region (unsigned,unsigned, double ,
	  double, vector<ca_callstacksample> &, libparaver::UIParaverTraceConfig *);

	static void do_analysis_presence_region_cube_tree (unsigned, unsigned,
		double, double, vector<ca_callstacksample> &,
		libparaver::UIParaverTraceConfig *, cube::Cube*, cube::Cnode *base);
	static void do_analysis_presence_region_cube_tree_r (
		TreeNodeHolder *tree, libparaver::UIParaverTraceConfig *pcf, 
		cube::Cube *cube, cube::Cnode *root);
};
#endif

#endif
