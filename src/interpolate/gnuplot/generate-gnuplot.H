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

#ifndef GNUPLOTGENERATOR_H_INCLUDED
#define GNUPLOTGENERATOR_H_INCLUDED

#include "interpolate.H"
#include "instance-container.H"
#include "instance-group.H"
#include "data-object.H"

using namespace std;

#include <stack>

class gnuplotGenerator
{
	public:
	static void gnuplot_single (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string &prefix,
	  const string &counter,
	  InterpolationResults *idata,
	  const string & TimeUnit,
	  const vector<DataObject*> & variables,
	  UIParaverTraceConfig *pcf);
	static string gnuplot_slopes (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string &prefix,
	  bool per_instruction,
	  const string & TimeUnit,
	  const vector<DataObject*> & variables,
	  UIParaverTraceConfig *pcf);
	static string gnuplot_model (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string & prefix,
	  const Model *m,
	  const string & TimeUnit,
	  const vector<DataObject*> & variables,
	  UIParaverTraceConfig *pcf);
	static string gnuplot_addresses_observed (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string & prefix,
	  const string & TimeUnit,
	  vector<DataObject*> & variables,
	  UIParaverTraceConfig *pcf);
	static string gnuplot_addresses (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string & prefix,
	  const string & TimeUnit,
	  vector<DataObject*> & variables,
	  UIParaverTraceConfig *pcf);
	static string gnuplot_addresses_cost (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string & prefix,
	  const string & TimeUnit,
	  vector<DataObject*> & variables,
	  UIParaverTraceConfig *pcf);
	static void gnuplot_groups (
	  InstanceContainer *ic,
	  const ObjectSelection *os,
	  const string & prefix,
	  StatisticType_t type);

	static string getStackStringDepth (
	  unsigned depth,
	  stack<CodeRefTriplet> scrt,
	  UIParaverTraceConfig *pcf);

	static void gnuplot_routine_part (
	  ofstream & gplot,
	  const string & fileDump,
	  InstanceGroup *ig,
	  UIParaverTraceConfig *pcf);
	static void gnuplot_addresses_part (
	  ofstream & gplot,
	  const string & fileDump,
	  InstanceGroup *ig,
	  const vector<DataObject*> & variables,
	  UIParaverTraceConfig *pcf);
};

#endif /* GNUPLOTGENERATOR_H_INCLUDED */
