/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  MPItrace                                 *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *                                                             ___           *
 *   +---------+     http:// www.cepba.upc.edu/tools_i.htm    /  __          *
 *   |    o//o |     http:// www.bsc.es                      /  /  _____     *
 *   |   o//o  |                                            /  /  /     \    *
 *   |  o//o   |     E-mail: cepbatools@cepba.upc.edu      (  (  ( B S C )   *
 *   | o//o    |     Phone:          +34-93-401 71 78       \  \  \_____/    *
 *   +---------+     Fax:            +34-93-401 25 77        \  \__          *
 *    C E P B A                                               \___           *
 *                                                                           *
 * This software is subject to the terms of the CEPBA/BSC license agreement. *
 *      You must accept the terms of this license to use this software.      *
 *                                 ---------                                 *
 *                European Center for Parallelism of Barcelona               *
 *                      Barcelona Supercomputing Center                      *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/folding/generate-gnuplot.H $
 | 
 | @last_commit: $Date: 2012-01-27 16:02:20 +0100 (dv, 27 gen 2012) $
 | @version:     $Revision: 948 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CALLSTACKANALYSIS_H_INCLUDED
#define CALLSTACKANALYSIS_H_INCLUDED

#include <vector>
#include <string>
#include "UIParaverTraceConfig.h"
#include <cube3/Cube.h>

using namespace std;

class ca_callstacksample
{
	public:
	float Time;
	unsigned Region;
	vector<unsigned> depth;
	vector<unsigned> caller;
	vector<unsigned> callerline;
};

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

	static void do_analysis (unsigned, string, vector<double> &, 
	  vector<ca_callstacksample> &, UIParaverTraceConfig *, cube::Cube*);
	static void do_analysis_presence_region (unsigned, string, unsigned, double ,
	  double, vector<ca_callstacksample> &, UIParaverTraceConfig *);
	static void do_analysis_presence_region_cube (unsigned, string, unsigned,
		double, double, vector<ca_callstacksample> &, UIParaverTraceConfig *,
		cube::Cube*, cube::Cnode *base);
};

#endif
