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

#include "prv-semantic-CSV.H"

#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <iostream>

PRVSemanticCSV::PRVSemanticCSV (const char *file)
{
	unsigned lineno = 0;

	ifstream f(file);
	if (f.is_open())
	{
		succeeded = true;

		string l;
		while (getline(f,l))
		{
			lineno++;

			istringstream iss(l);
			vector<string> tokens;
			copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string> >(tokens));

			if (tokens.size() != 4)
			{
				succeeded = false;
				cerr << "File " << file << " does not seem to be well formatted at line " << lineno << endl;
				break;
			}

			string Object = tokens[0];
			unsigned ptask, task, thread;
			if (common::decomposePtaskTaskThread (Object, ptask, task, thread))
			{
				ObjectSelection os(ptask, task, thread);
				unsigned long long Time = atoll (tokens[1].c_str());
				unsigned long long Duration = atoll (tokens[2].c_str());
				string Value = tokens[3];

				if (Time >= 0 && Duration >= 0)
				{
					PRVSemanticValue *sv = new PRVSemanticValue (Time, Duration, Value);

					vector<PRVSemanticValue*> v;
					if (SemanticsPerObject.count(os) > 0)
						v = SemanticsPerObject[os];
					v.push_back (sv);
					SemanticsPerObject[os] = v;
				}
			}
		}
		f.close();
	}
	else
		succeeded = false;
}

vector<PRVSemanticValue*> PRVSemanticCSV::getSemantics (unsigned ptask, unsigned task, unsigned thread)
{
	ObjectSelection o(ptask, task, thread);
	return getSemantics(o);
}

vector<PRVSemanticValue*> PRVSemanticCSV::getSemantics (const ObjectSelection &o)
{
	if (SemanticsPerObject.count(o) == 0)
	{
		vector<PRVSemanticValue*> empty;
		return empty;
	}
	else
		return SemanticsPerObject[o];
}

/*
int main (int argc, char *argv[])
{
	PRVSemanticCSV *csv = new PRVSemanticCSV (argv[1]);

	vector<PRVSemanticValue*> v111 = csv->getSemantics (1,1,1);
	for (unsigned s = 0; s < v111.size(); s++)
		cout << "1.1.1 From: " << v111[s]->getTime() << " To: " << v111[s]->getTo() << " Value: " << v111[s]->getValue() << endl;

	vector<PRVSemanticValue*> v121 = csv->getSemantics (1,2,1);
	for (unsigned s = 0; s < v121.size(); s++)
		cout << "1.2.1 From: " << v121[s]->getTime() << " To: " << v121[s]->getTo() << " Value: " << v121[s]->getValue() << endl;

	vector<PRVSemanticValue*> v112 = csv->getSemantics (1,1,2);
	for (unsigned s = 0; s < v112.size(); s++)
		cout << "1.1.2 From: " << v112[s]->getTime() << " To: " << v112[s]->getTo() << " Value: " << v112[s]->getValue() << endl;

	delete csv;
}
*/
