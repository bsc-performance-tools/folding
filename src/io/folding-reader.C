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

#include "common.H"

#include "folding-reader.H"
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>

void FoldingReader::ReadSamples (const string & filenameextract,
	const ObjectSelection *os, const string & TimeUnit,
	set<string> &allcounters, set<string> &allregions,
	vector<Instance*> &Instances, ObjectSelection *osfeed,
	vector<Instance*> &feedInstances)
{
	Instance *i = NULL;
	char type;

	cout << "Reading data belonging to Paraver Object " << os->toString()
	  << " from file '" << filenameextract << "'" << endl;

	ifstream file (filenameextract.c_str());
	if (!file.is_open())
	{
		cerr << "Unable to open file named " << filenameextract << endl;
		exit (-1);
	}

	vector< map<unsigned, unsigned> > vcallers;

	/* Calculate totals and number of presence of each region */
	while (true)
	{
		file >> type;

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

			set<unsigned> dataobjects;
			string dataobjectsheader;
			file >> dataobjectsheader;
			if (dataobjectsheader == "DO")
			{
				unsigned ndataobjects;
				file >> ndataobjects;
				while (ndataobjects > 0)
				{
					unsigned tmp;
					file >> tmp;
					dataobjects.insert (tmp);
					ndataobjects--;
				}
			}
			else
			{
				cerr << "Fatal error! Cannot locate DataObject header for instance" << endl;
				exit (-1);
			}

			i = new Instance (ptask, task, thread, RegionName, startTime,
			  duration, icounters, itotalcountervalues, dataobjects);
			if (i == NULL)
			{
				cerr << "Fatal error! Cannot allocate memory for a new instance!" << endl;
				exit (-1);
			}
		}
		else if (type == 'S')
		{
			/* Example of Sample without address*/
			/* S 36145340222 36433223 5 UNHALTED_CORE_CYCLES 94368325 INSTRUCTION_RETIRED 66006147 RESOURCE_STALLS:FCSW 0 2 0 3 9 9 1 4 3 3 0 */
			/* S time #counters <counter name, counter value> #code triplets <depth, caller, caller line, caller line AST 0*/
			/* Example of Sample with address*/
			/* S 36145340222 36433223 5 UNHALTED_CORE_CYCLES 94368325 INSTRUCTION_RETIRED 66006147 RESOURCE_STALLS:FCSW 0 2 0 3 9 9 1 4 3 3 Y 1000 2 2 1*/
			/* S time #counters <counter name, counter value> #code triplets <depth, caller, caller line, caller line AST 0 address object mem-level tlb-level*/

			assert (i != NULL);

			Sample *s;

			unsigned long long sTime, iTime;

			file >> sTime;
			file >> iTime;

			map<string, unsigned long long> icv;
			unsigned ncounters;
			file >> ncounters;
			while (ncounters > 0)
			{
				string countername;
				unsigned long long countervalue;
				file >> countername;
				file >> countervalue;
				icv[countername] = countervalue;
				ncounters--;
			}

			map<unsigned, unsigned> callers;
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
				callers[depth] = c;
				ncrt--;
			}

			char hasaddress;
			string referencetype;
			unsigned long long ar = 0;
			unsigned long long ar_allocatedobject = 0;
			char hasaddressinfo = 0;
			unsigned ar_mem_level = 0;
			unsigned ar_tlb_level = 0;
			unsigned cycles_cost = 0;
			file >> hasaddress;

			if (hasaddress == 'Y')
			{
				file >> referencetype;
				file >> ar;
				file >> ar_allocatedobject;
				file >> hasaddressinfo;
				if (hasaddressinfo == 'Y')
				{
					file >> ar_mem_level;
					file >> ar_tlb_level;
					file >> cycles_cost;
				}
			}

			/* Ignore addresses from the high part of the address space (48 bit out of 64) */
			if (hasaddress && !(ar & 0xFFFF800000000000))
			{
				AddressReferenceType_t rt = LOAD;
				if (referencetype == "ST")
					rt = STORE;

				if (hasaddressinfo)
					s = new Sample (sTime, iTime, icv, ct, rt, ar,
					  ar_allocatedobject, ar_mem_level, ar_tlb_level,
					  cycles_cost);
				else
					s = new Sample (sTime, iTime, icv, ct, rt, ar,
					  ar_allocatedobject);
			}
			else
				s = new Sample (sTime, iTime, icv, ct);

			bool found = false;
			for (unsigned v = 0; v < vcallers.size() && !found; v++)
				if (vcallers[v] == callers)
				{
					found = true;
					s->setCallersId (v);
				}
			if (!found)
			{
				s->setCallersId (vcallers.size());
				vcallers.push_back (callers);
			}

			if (i == NULL)
			{
				cerr << "Fatal error! Cannot allocate memory for a new sample!" << endl;
				exit (-1);
			}

			/* Skip samples that do not contain timeunit selected if it is not default */
			if (common::DefaultTimeUnit == TimeUnit || icv.count (TimeUnit) > 0)
			{
				s->normalizeData (i->getDuration(), i->getTotalCounterValues(), TimeUnit);

				if (ncodereftriplets > 0)
					s->processCodeTriplets ();

				i->addSample (s);
			}
			else
				delete s;
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


void FoldingReader::ReadVariables (const string & filenameextract,
	vector<DataObject*> &dataObjects)
{
	string filename = filenameextract.substr (0, filenameextract.rfind (".extract"))
	  + ".dataobjects";

	cout << "Reading variable info from file '" << filename << "'" << endl;

	ifstream file (filename.c_str());
	if (!file.is_open())
	{
		cerr << "Unable to open file named " << filename << endl;
		exit (-1);
	}

	while (true)
	{
		string type, name, s_start, s_end;

		file >> type;
		if (file.eof())
			break;
		file >> name;
		file >> s_start;
		file >> s_end;

		unsigned long long ull_start, ull_end;
		stringstream ss_start, ss_end;
		ss_start << hex << s_start;
		ss_start >> ull_start;
		ss_end << hex << s_end;
		ss_end >> ull_end;

		DataObject *DO = NULL;
		if (type == "S")
			DO = new DataObject_static (ull_start, ull_end, name);
		else if (type == "D")
			DO = new DataObject_dynamic (ull_start, ull_end, name);

		if (DO != NULL)
			dataObjects.push_back (DO);
	}
}

