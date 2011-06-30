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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/folding/region-analyzer.H $
 | 
 | @last_commit: $Date: 2009-12-02 12:20:56 +0100 (dc, 02 des 2009) $
 | @version:     $Revision: 61 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef FEEDTRACE_WR_H_INCLUDED
#define FEEDTRACE_WR_H_INCLUDED

#include "region-info.H"

#include <vector>
#include <list>
#include <string>
#include "UIParaverTraceConfig.h"

using namespace std;
using namespace libparaver;

void SearchForRegionsWithinRegion (string tracename, unsigned task, unsigned thread,
	unsigned long long Type, unsigned long long TimeType, unsigned long long TimeValue,
	unsigned long long *out_Tstart, unsigned long long *out_Tend,
	vector<string> &vCounters, RegionInfo &regions,
	UIParaverTraceConfig *pcf);

#endif
