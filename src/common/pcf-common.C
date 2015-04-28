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

#include "pcf-common.H"
#include "common.H"

#include <algorithm>

#define PAPI_MIN_COUNTER   42000000
#define PAPI_MAX_COUNTER   42999999

unsigned pcfcommon::lookForCounter (string &name, UIParaverTraceConfig *pcf)
{
	vector<unsigned> vtypes = pcf->getEventTypes();

	for (const auto & type : vtypes)
		if (type >= PAPI_MIN_COUNTER && type <= PAPI_MAX_COUNTER)
		{
			string s = pcf->getEventType (type);
			if (s.length() > 0)
			{
				string tmp = s.substr (0, s.find (' '));
				if (tmp[0] == '(' && tmp[tmp.length()-1] == ')')
					tmp = tmp.substr (1, tmp.length()-2);
				if (tmp == name)
					return type;
			}
		}

	return 0;
}

unsigned pcfcommon::lookForValueString (UIParaverTraceConfig *pcf, unsigned type, string str, bool &found)
{
	found = false;

	vector<unsigned> vtypes = pcf->getEventTypes();
	if (find (vtypes.begin(), vtypes.end(), type) != vtypes.end())
	{
		vector<unsigned> v;
		try
		{
			v = pcf->getEventValues(type);
		}
		catch (...)
		{
			found = false;
			return 0;
		}
		for (unsigned i = 0; i < v.size(); i++)
		{
			string evstr = pcf->getEventValue (type, v[i]);
			if (str == evstr)
			{
				found = true;
				return v[i];
			}
			else if (str == common::removeSpaces (evstr))
			{
				found = true;
				return v[i];
			}
		}
	}

	return 0;
}

void pcfcommon::lookForCallerLineInfo (UIParaverTraceConfig *pcf, unsigned id,
	string &file, unsigned &line)
{
	string cl = pcf->getEventValue (30000100, id);
	line = atoi ((cl.substr (0, cl.find (" "))).c_str());
	int pos_open = cl.find ("(");
	int pos_close = cl.find (")");
	file = cl.substr (pos_open+1, pos_close-pos_open-1);
}

void pcfcommon::lookForCallerASTInfo (UIParaverTraceConfig *pcf, unsigned caller,
	unsigned callerlineast, string &routine, string &file, unsigned &bline,
	unsigned &eline)
{
	string cl = pcf->getEventValue (30000200, callerlineast);

	bline = 0;
	eline = 0;

	/* Check whether we have a line separator or not */

	int sep_position, par_position = cl.find ("(");
	string tmp = cl.substr (0, par_position);
	if ((sep_position = tmp.find ("-")) != string::npos)
	{
		string tmp1 = tmp.substr (0, sep_position);
		string tmp2 = tmp.substr (sep_position+1);
		bline = atoi (tmp1.c_str());
		eline = atoi (tmp2.c_str());
	}
	else
	{
		/* If can't find -, then there isn't a separator */
		bline = eline = atoi (tmp.c_str());
	}

	int pos_open = par_position;
	int pos_close = cl.find (")");
	file = cl.substr (pos_open+1, pos_close-pos_open-1);
	routine = pcf->getEventValue (30000000, caller);
}

void pcfcommon::lookForCallerFullInfo (UIParaverTraceConfig *pcf, unsigned caller,
	  unsigned callerline, unsigned callerlineast, string &routine, string &file,
	  unsigned &line, unsigned &bline, unsigned &eline)
{
	lookForCallerASTInfo (pcf, caller, callerlineast, routine, file, bline,
	  eline);
	lookForCallerLineInfo (pcf, callerline, file, line);
}

