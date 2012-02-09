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
 | @file: $HeadURL$
 | 
 | @last_commit: $Date$
 | @version:     $Revision$
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id$";

#include "common.H"

#include <sstream>
#include <iomanip>

#define PAPI_MIN_COUNTER   42000000
#define PAPI_MAX_COUNTER   42009998

string common::convertDouble (double d, int i)
{
	stringstream s;//create a stringstream
	s << fixed << setprecision(i) << d;//add number to the stream with selected precision
	return s.str();//return a string with the contents of the stream
}

string common::convertInt (int i)
{
	stringstream s;//create a stringstream
	s << i;//add number to the stream
	return s.str();//return a string with the contents of the stream
}

string common::removeSpaces (string &in)
{
	string tmp = in;
	for (string::iterator it = tmp.begin(); it != tmp.end(); it++)
		if (*it == ' ')
			*it = '_';
	return tmp;
}

unsigned common::lookForCounter (string &name, UIParaverTraceConfig *pcf)
{
	/* Look for every counter in the vector its code within the PCF file */
	for (unsigned j = PAPI_MIN_COUNTER; j <= PAPI_MAX_COUNTER; j++)
	{
		string s = pcf->getEventType (j);
		if (s.length() > 0)
		{
			string tmp = s.substr (s.find ('(')+1, s.rfind (')') - (s.find ('(') + 1));
			if (tmp == name)
				return j;
		}
	}
	return 0;
}

