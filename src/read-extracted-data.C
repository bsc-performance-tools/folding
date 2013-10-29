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

#include "read-extracted-data.H"
#include <iostream>
#include <fstream>
#include <assert.h>

void ReadExtractData::ReadDataFromFile (string filename, ObjectSelection *os,
	set<string> &allcounters, set<string> &allregions,
	vector<Instance*> &Instances, ObjectSelection *osfeed,
	vector<Instance*> &feedInstances)
{
	Instance *i = NULL;
	int line = 0;
	char type;

	cout << "Reading data belonging to Paraver Object " << os->toString()
	  << " from file '" << filename << "'" << endl;

	ifstream file (filename.c_str());
	if (!file.is_open())
	{
		cerr << "Unable to open file named " << filename << endl;
		exit (-1);
	}

	/* Calculate totals and number of presence of each region */
	while (true)
	{
		file >> type;
		line++;

		if (file.eof())
			break;

		if (type == 'I')
		{
			/* Example of Instance */
			/* I 1 1 1 main 36108906999 145894729 3 UNHALTED_CORE_CYCLES 377893506 INSTRUCTION_RETIRED 240152588 RESOURCE_STALLS:FCSW 0 */
			/* I ptask task thread RegionName startTime duration #counters <counter_name, total counter value > */

			unsigned ncounters;

			if (i != NULL && i->getNumSamples() > 0)
			{
				bool used = false;

				if (os->match (i->getptask(), i->gettask(), i->getthread()))
				{
					Instances.push_back (i);
					used = true;
				}
				if (osfeed != NULL)
					if (osfeed->match (i->getptask(), i->gettask(), i->getthread()))
					{
						feedInstances.push_back (i);
						used = true;
					}

				/* Should we remove also the samples allocated? */ 
				if (!used)
					delete i;
			}


			unsigned ptask, task, thread;
			string RegionName;
			unsigned long long startTime, duration;

			file >> ptask;
			file >> task;
			file >> thread;
			file >> RegionName;
			allregions.insert (RegionName);
			file >> startTime;
			file >> duration;

			set<string> icounters;
			map<string, unsigned long long> itotalcountervalues;
			file >> ncounters;
			while (ncounters > 0)
			{
				string countername;
				unsigned long long countervalue;
				file >> countername;
				allcounters.insert (countername);
				icounters.insert (countername);
				file >> countervalue;
				itotalcountervalues[countername] = countervalue;
				ncounters--;
			}

			i = new Instance (ptask, task, thread, RegionName, startTime,
			  duration, icounters, itotalcountervalues);
			if (i == NULL)
			{
				cerr << "Fatal error! Cannot allocate memory for a new instance!" << endl;
				exit (-1);
			}
		}
		else if (type == 'S')
		{
			/* Example of Sample */
			/* S 36145340222 36433223 5 UNHALTED_CORE_CYCLES 94368325 INSTRUCTION_RETIRED 66006147 RESOURCE_STALLS:FCSW 0 2 0 3 9 9 1 4 3 3 */
			/* S time #counters <counter name, counter value> #code triplets <depth, caller, caller line, caller line AST */

			assert (i != NULL);

			unsigned long long sTime, iTime;
			double nTime;

			file >> sTime;
			file >> iTime;
			nTime = ((double) iTime) / ((double) i->getDuration());

			map<string, unsigned long long> icv;
			map<string, double> ncv;
			unsigned ncounters;
			file >> ncounters;
			while (ncounters > 0)
			{
				string countername;
				unsigned long long countervalue;
				file >> countername;
				file >> countervalue;
				icv[countername] = countervalue;
				ncv[countername] =
				  ((double) countervalue / (double) i->getTotalCounterValue (countername));
				ncounters--;
			}

			map<unsigned, CodeRefTriplet> ct;
			unsigned ncodereftriplets, ncrt;
			file >> ncodereftriplets;
			ncrt = ncodereftriplets;
			while (ncrt > 0)
			{
				unsigned depth;
				unsigned c, cl, clast;

				file >> depth;
				file >> c;
				file >> cl;
				file >> clast;
				CodeRefTriplet triplet (c, cl, clast);
				ct[depth] = triplet;
				ncrt--;
			}

			Sample *s = new Sample (sTime, iTime, nTime, icv, ncv, ct);
			if (i == NULL)
			{
				cerr << "Fatal error! Cannot allocate memory for a new sample!" << endl;
				exit (-1);
			}

			if (ncodereftriplets > 0)
				s->processCodeTriplets ();

			i->addSample (s);
		}
	}

	if (i != NULL && i->getNumSamples() > 0)
	{
		bool used = false;

		if (os->match (i->getptask(), i->gettask(), i->getthread()))
		{
			Instances.push_back (i);
			used = true;
		}
		if (osfeed != NULL)
			if (osfeed->match (i->getptask(), i->gettask(), i->getthread()))
			{
				feedInstances.push_back (i);
				used = true;
			}

		/* Should we remove also the samples allocated? */ 
		if (!used)
			delete i;
	}

	file.close();
}


