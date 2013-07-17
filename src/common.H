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

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "folding-config.h"
#include <string>
#include "UIParaverTraceConfig.h"

#define UNREFERENCED(a) ((a) = (a))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

using namespace std;
using namespace libparaver;

class common
{
	public:
	static string convertInt (int);
	static string convertDouble (double, int);
	static string removeSpaces (string &in);
	static unsigned lookForCounter (string &name, UIParaverTraceConfig *pcf);
	static bool existsFile (string file);
	static bool existsDir (string dir);
	static void lookForCallerLineInfo (UIParaverTraceConfig *pcf,
	  unsigned id, string &file, int &line);
	static void lookForFullCallerInfo (UIParaverTraceConfig *pcf, unsigned caller,
	  unsigned callerlineast, string &routine, string &file, int &bline, int &eline);
	static void CleanMetricsDirectory (string &directory);
	static void CleanMetricsDirectory_r (char *directory);
	static bool isMIPS (string s);
	static bool DEBUG();
	static bool decomposePtaskTaskThread (string &s, unsigned &ptask,
	  unsigned &task, unsigned &thread);
	static bool decomposePtaskTaskThreadWithAny (string &s, unsigned &ptask,
	  bool &ptaskany, unsigned &task, bool &taskany, unsigned &thread, bool &threadany);
	static unsigned lookForValueString (UIParaverTraceConfig *pcf, unsigned type, string str, bool &found);
	static string basename (string s);
};

#endif
