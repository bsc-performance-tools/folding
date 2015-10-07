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
#include "prv-types.H"

#include <algorithm>
#include <sstream>

unsigned pcfcommon::lookForCounter (const string &name, UIParaverTraceConfig *pcf)
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

unsigned pcfcommon::lookForValueString (UIParaverTraceConfig *pcf,
	unsigned type, string str, bool &found)
{
	found = false;

	if ((type >= EXTRAE_SAMPLE_CALLER_MIN && type <= EXTRAE_SAMPLE_CALLER_MAX)
	  || type == EXTRAE_USER_FUNCTION)
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
			string r;
			lookForCallerInfo (pcf, v[i], r);
			if (r == str)
			{
				found = true;
				return v[i];
			}
		}
	}
	else if ((type >= EXTRAE_SAMPLE_CALLERLINE_MIN && type <= EXTRAE_SAMPLE_CALLERLINE_MAX)
	  || type == EXTRAE_USER_FUNCTION_LINE)
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
			string file;
			unsigned line;
			stringstream ss;
			lookForCallerLineInfo (pcf, v[i], file, line);
			ss << line << "_" << file;
			if (ss.str() == str)
			{
				found = true;
				return v[i];
			}
		}
	}
	else
	{
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
	}

	return 0;
}

void pcfcommon::lookForCallerInfo (UIParaverTraceConfig *pcf, unsigned id,
	string &routine)
{
	/* Example: NEW CalcElem..ivatives [CalcElemShapeFunctionDerivatives]
                    Function
                OLD CalcElemShapeFunctionDerivatives */
	string cl = pcf->getEventValue (EXTRAE_SAMPLE_CALLER, id);

	if (cl.find ("[") != string::npos)
	{
		int pos_begin = cl.find ("[");
		int pos_end = cl.find ("]");
		routine = cl.substr (pos_begin+1, pos_end-pos_begin-1);
	}
	else
		routine = cl;
}
void pcfcommon::lookForCallerLineInfo (UIParaverTraceConfig *pcf, unsigned id,
	string &file, unsigned &line)
{
	/* Example: NEW 178 (stream.simple.c, stream.simple) 
                    Line file             binary
                OLD 178 (stream.simple.c) [stream.simple]
                OR
                    END? */
	string cl = pcf->getEventValue (EXTRAE_SAMPLE_CALLERLINE, id);

	if (cl.find (" ") != string::npos)
	{
		string afterspace = cl.substr (cl.find(" "));
		line = atoi ((cl.substr (0, cl.find (" "))).c_str());
	
		int pos_begin, pos_end;
		bool newformat = afterspace.find (",") != string::npos;
		if (newformat)
		{
			pos_begin = afterspace.find ("(");
			pos_end = afterspace.find (",");
		}
		else
		{
			pos_begin = afterspace.find ("(");
			pos_end = afterspace.find (")");
		}
	
		file = afterspace.substr (pos_begin+1, pos_end-pos_begin-1);
	}
	else
	{
		file = "";
		line = 0;
	}
}

void pcfcommon::lookForCallerASTInfo (UIParaverTraceConfig *pcf, unsigned caller,
	unsigned callerlineast, string &routine, string &file, unsigned &bline,
	unsigned &eline)
{
	/* Example: NEW
                178 (stream.simple.c, stream.simple) 
                Line file             binary 
       OR
                179-180 (stream.simple.c)

                OLD
                178 (stream.simple.c) [stream.simple]
       OR
                179-180 (stream.simple.c) */

	string cl = pcf->getEventValue (EXTRAE_SAMPLE_CALLERLINE_AST, callerlineast);

	routine = pcf->getEventValue (EXTRAE_SAMPLE_CALLER, caller);

	bline = 0;
	eline = 0;
	file = "";

	if (cl.find (" ") != string::npos)
	{
		/* Check whether we have a line separator or not */
	
		string beforespace = cl.substr (0, cl.find (" "));
		string afterspace = cl.substr (cl.find(" "));
	
		int pos_begin, pos_end;
		int sep_position = beforespace.find ("-");
		int par_position = afterspace.find (" ");
		bool has_range = sep_position != string::npos;
		if (has_range)
		{
			string tmp1 = beforespace.substr (0, sep_position);
			string tmp2 = beforespace.substr (sep_position+1);
			bline = atoi (tmp1.c_str());
			eline = atoi (tmp2.c_str());
			pos_begin = afterspace.find ("(");
			pos_end = afterspace.find (")");
		}
		else
		{
			/* If can't find -, then there isn't a separator */
			bline = eline = atoi (beforespace.c_str());
			bool newformat = afterspace.find (",") != string::npos;
			if (newformat)
			{
				pos_begin = afterspace.find ("(");
				pos_end = afterspace.find (",");
			}
			else
			{
				pos_begin = afterspace.find ("(");
				pos_end = afterspace.find (")");
			}
		}
	
		file = afterspace.substr (pos_begin+1, pos_end-pos_begin-1);
	}
}

void pcfcommon::lookForCallerFullInfo (UIParaverTraceConfig *pcf, unsigned caller,
	  unsigned callerline, unsigned callerlineast, string &routine, string &file,
	  unsigned &line, unsigned &bline, unsigned &eline)
{
	lookForCallerASTInfo (pcf, caller, callerlineast, routine, file, bline,
	  eline);
	lookForCallerLineInfo (pcf, callerline, file, line);
}

