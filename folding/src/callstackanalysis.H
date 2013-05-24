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

#include "common.H"
#include <vector>
#include <string>

using namespace std;

class ca_callstacksample
{
	public:
	float Time;
	unsigned Region;
	unsigned Instance;
//	vector<unsigned> depth;
	vector<unsigned long long> caller;
	vector<unsigned long long> callerline;
	vector<unsigned long long> callerType;
	vector<unsigned long long> callerlineType;
	vector<pair<unsigned long long, unsigned long long > > callerlineASTBlock; /* first will be begin, second will be end */
};


#include "UIParaverTraceConfig.h"
#include "treenodeholder.H"

#if HAVE_CUBE
# include <cube3/Cube.h>

class TreeNodeHolder;


class ca_callstackanalysis_presence
{
	public:
	unsigned occurrences;
	vector<unsigned long long> caller;
	vector<unsigned long long> callerline;
};

class ca_callstackanalysis
{
	public:
	static unsigned do_analysis (
	  string prefix, string countercode, string counter, double value, unsigned R,
	  string Rstr, vector<double> &breakpoints, vector<ca_callstacksample> &samples,
	  UIParaverTraceConfig *pcf, cube::Cube *cubev, string &sourceDir);
#if  0
	static void do_analysis_presence_region (
	  unsigned R, unsigned phase,	double from, double to,
	  vector<ca_callstacksample> &samples, UIParaverTraceConfig *pcf);
	static void do_analysis_presence_region_cube_tree_r (
	  TreeNodeHolder *tree, UIParaverTraceConfig *pcf, cube::Cube *cubev,
	  cube::Cnode *root, string &sourceDir);
	static void do_analysis_presence_region_cube_tree (
	  unsigned R, unsigned phase,	double from, double to,
	  vector<ca_callstacksample> &samples, UIParaverTraceConfig *pcf,
	  cube::Cube *cubev, cube::Cnode *root, string &sourceDir);
#endif
	static void do_analysis_presence_region_cube_tree (
	  TreeNodeHolder *tree, UIParaverTraceConfig *pcf, cube::Cube *cubev,
		cube::Cnode *root, cube::Metric *m, string &sourceDir,
		string &metricName);
	static void spread_severity_tree (
		TreeNodeHolder *tree, UIParaverTraceConfig *pcf, cube::Cube *cubev,
	  cube::Cnode *root, cube::Metric *m, double severity, string &sourceDir,	
		string &metricName);
	static void spread_constant_tree (
		TreeNodeHolder *tnh, UIParaverTraceConfig *pcf, double constant,
			string &sourceDir, string &Name);
};
#endif

#endif
