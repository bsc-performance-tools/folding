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

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

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
	private:
	static unsigned numDigits (unsigned long long v, unsigned base);

	public:
	static const string DefaultTimeUnit;

	static unsigned numDecimalDigits (unsigned long long v);
	static unsigned numHexadecimalDigits (unsigned long long v);

	static unsigned getNumCores (bool &found);
	static unsigned getNumProcessors (bool &found);

	static string convertInt (int);
	static string convertInt (unsigned);
	static string convertInt (size_t st);
	static string convertDouble (double, int);

	static string removeSpaces (const string &in);
	static string removeUnwantedChars (const string &in);

	static bool existsFile (const string & file);
	static bool existsDir (const string & dir);

	static void CleanMetricsDirectory (const string & directory);
	static void CleanMetricsDirectory_r (const char *directory);

	static bool isMIPS (const string & s);

	static bool DEBUG();

	static bool decomposePtaskTaskThread (const string &s, unsigned &ptask,
	  unsigned &task, unsigned &thread);
	static bool decomposePtaskTaskThreadWithAny (const string &s, unsigned &ptask,
	  bool &ptaskany, unsigned &task, bool &taskany, unsigned &thread, bool &threadany);
	static string basename (const string & s);

	static bool addressInStack (unsigned long long address)
	  { return address >= 0x80000000; }
};

#endif
