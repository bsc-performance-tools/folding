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

#ifndef GNUPLOTGENERATORREFERENCES_H_INCLUDED
#define GNUPLOTGENERATORREFERENCES_H_INCLUDED

#include "interpolate.H"
#include "instance-container.H"
#include "instance-group.H"
#include "data-object.H"

using namespace std;

#include <stack>

class gnuplotGeneratorReferences
{
	public:

	static void generate (
	  double verticalspace,
	  double verticalorigin,
	  ofstream & gplot,
	  const string & fileDump,
	  InstanceGroup *ig,
	  const vector<DataObject*> & variables,
	  UIParaverTraceConfig *pcf);
};

#endif /* GNUPLOTGENERATORREFERENCES_H_INCLUDED */
