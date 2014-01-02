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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/common/common.H $
 | 
 | @last_commit: $Date: 2013-10-29 13:39:46 +0100 (mar, 29 oct 2013) $
 | @version:     $Revision: 2259 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef PCFCOMMON_H_INCLUDED
#define PCFCOMMON_H_INCLUDED

#include "folding-config.h"
#include <string>
#include "UIParaverTraceConfig.h"

using namespace std;
using namespace libparaver;

class pcfcommon
{
	public:
	static unsigned lookForCounter (string &name, UIParaverTraceConfig *pcf);

	static void lookForCallerLineInfo (UIParaverTraceConfig *pcf,
	  unsigned id, string &file, unsigned &line);
	static void lookForCallerASTInfo (UIParaverTraceConfig *pcf, unsigned caller,
	  unsigned callerlineast, string &routine, string &file, unsigned &bline, unsigned &eline);
	static void lookForCallerFullInfo (UIParaverTraceConfig *pcf, unsigned caller,
	  unsigned callerline, unsigned callerlineast, string &routine, string &file,
	  unsigned &line, unsigned &bline, unsigned &eline);
	static unsigned lookForValueString (UIParaverTraceConfig *pcf, unsigned type,
	  string str, bool &found);
};

#endif
