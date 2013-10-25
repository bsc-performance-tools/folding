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

static char __attribute__ ((unused)) rcsid[] = "$Id$";

#include "common.H"

#include "ParaverTrace.h"
#include "ParaverTraceThread.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceApplication.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include "interpolate.H"

using namespace::libparaver;
using namespace::std;

int main (int argc, char *argv[])
{
	string prvFile;

	if (argc != 2)
		return -1;

	prvFile = argv[1];
	UIParaverTraceConfig *pcf = new UIParaverTraceConfig;
	pcf->parse (prvFile.substr (0, prvFile.length()-3) + string ("pcf"));

	vector<unsigned> vtypes = pcf->getEventTypes();
	for (unsigned u = 0; u < vtypes.size(); u++)
	{
		bool unusable =
		  ( vtypes[u] >= PAPI_MIN_COUNTER && vtypes[u] <= PAPI_MAX_COUNTER ) ||
		  ( vtypes[u] == PAPI_CHANGE_COUNTER_SET ) ||
		  ( vtypes[u] >= FOLDED_BASE ) ||
		  ( vtypes[u] >= EXTRAE_SAMPLE_CALLER_MIN && vtypes[u] <= EXTRAE_SAMPLE_CALLERLINE_AST_MAX);

		if (!unusable)
		{
			string description = pcf->getEventType (vtypes[u]);
			cout << vtypes[u] << ";" << description << endl;
		}
	}

	return 0;
}

