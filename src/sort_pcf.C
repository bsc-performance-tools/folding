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

#include <UIParaverTraceConfig.h>
#include <fstream>
#include <iostream>
#include <list>
#include <stdlib.h>
#include <stdio.h>

#include "translate_prv.H"

using namespace std;
using namespace libparaver;

class Information
{
	public:
	string file_name;
	unsigned line;
	unsigned value;

	Information (string &file_name, unsigned line, unsigned value);
	~Information ();
};

Information::Information (string &file_name, unsigned line, unsigned value)
{
	this->file_name = file_name;
	this->line = line;
	this->value = value;
}

Information::~Information ()
{
}

list<Information> SortEvents (UIParaverTraceConfig *pcf)
{
	list<Information> tmp;
	vector<unsigned> v = pcf->getEventValues (30000100);

	unsigned i = 2;
	while (i != v.size())
	{
		string ev = pcf->getEventValue(30000100, i);
		unsigned line = atoi ((ev.substr (0, ev.find(' '))).c_str());
		string file_name = ev.substr(ev.find('(')+1,ev.find(')')-ev.find ('(')-1);

		list<Information>::iterator it = tmp.begin();
		bool inserted = false;
		while (it != tmp.end() && !inserted)
		{
			if ((*it).file_name > file_name)
			{
				tmp.insert (it, *(new Information(file_name, line, i)));
				inserted = true;
			}
			else if ((*it).file_name == file_name && (*it).line > line)
			{
				tmp.insert (it, *(new Information(file_name, line, i)));
				inserted = true;
			}
			
			it++;
		}
		if (!inserted)
			tmp.push_back (*(new Information(file_name, line, i)));

		i++;
	}
	return tmp;
}

void DefaultPCFdata (ofstream &pcf_new_file)
{
	/* Default PCF data */
	pcf_new_file
	<< "DEFAULT_OPTIONS" << endl
	<< "LEVEL               THREAD" << endl
	<< "UNITS               NANOSEC" << endl
	<< "LOOK_BACK           100" << endl
	<< "SPEED               1" << endl
	<< "FLAG_ICONS          ENABLED" << endl
	<< "NUM_OF_STATE_COLORS 1000" << endl
	<< "YMAX_SCALE          37" << endl
	<< endl
	<< "DEFAULT_SEMANTIC" << endl
	<< "THREAD_FUNC          State As Is" << endl
	<< endl
	<< "STATES" << endl
	<< "0    Idle" << endl
	<< "1    Running" << endl
	<< "2    Not created" << endl
	<< "3    Waiting a message" << endl
	<< "4    Blocking Send" << endl
	<< "5    Synchronization" << endl
	<< "6    Test/Probe" << endl
	<< "7    Scheduling and Fork/Join" << endl
	<< "8    Wait/WaitAll" << endl
	<< "9    Blocked" << endl
	<< "10    Immediate Send" << endl
	<< "11    Immediate Receive" << endl
	<< "12    I/O" << endl
	<< "13    Group Communication" << endl
	<< "14    Tracing Disable" << endl
	<< "15    Others" << endl
	<< "16    Send Receive" << endl
	<< endl
	<< "STATES_COLOR" << endl
	<< "0    {117,195,255}" << endl
	<< "1    {0,0,255}" << endl
	<< "2    {255,255,255}" << endl
	<< "3    {255,0,0}" << endl
	<< "4    {255,0,174}" << endl
	<< "5    {179,0,0}" << endl
	<< "6    {0,255,0}" << endl
	<< "7    {255,255,0}" << endl
	<< "8    {235,0,0}" << endl
	<< "9    {0,162,0}" << endl
	<< "10    {255,0,255}" << endl
	<< "11    {100,100,177}" << endl
	<< "12    {172,174,41}" << endl
	<< "13    {255,144,26}" << endl
	<< "14    {2,255,177}" << endl
	<< "15    {192,224,0}" << endl
	<< "16    {66,66,66}" << endl
	<< endl
	<< "GRADIENT_COLOR" << endl
	<< "0    {0,255,2}" << endl
	<< "1    {0,244,13}" << endl
	<< "2    {0,232,25}" << endl
	<< "3    {0,220,37}" << endl
	<< "4    {0,209,48}" << endl
	<< "5    {0,197,60}" << endl
	<< "6    {0,185,72}" << endl
	<< "7    {0,173,84}" << endl
	<< "8    {0,162,95}" << endl
	<< "9    {0,150,107}" << endl
	<< "10    {0,138,119}" << endl
	<< "11    {0,127,130}" << endl
	<< "12    {0,115,142}" << endl
	<< "13    {0,103,154}" << endl
	<< "14    {0,91,166}" << endl
	<< endl
	<< "GRADIENT_NAMES" << endl
	<< "0    Gradient 0" << endl
	<< "1    Grad. 1/MPI Events" << endl
	<< "2    Grad. 2/OMP Events" << endl
	<< "3    Grad. 3/OMP locks" << endl
	<< "4    Grad. 4/User func" << endl
	<< "5    Grad. 5/User Events" << endl
	<< "6    Grad. 6/General Events" << endl
	<< "7    Grad. 7/Hardware Counters" << endl
	<< "8    Gradient 8" << endl
	<< "9    Gradient 9" << endl
	<< "10    Gradient 10" << endl
	<< "11    Gradient 11" << endl
	<< "12    Gradient 12" << endl
	<< "13    Gradient 13" << endl
	<< "14    Gradient 14" << endl << endl;
}

void generatePCFfile (string PCFfile, list<Information> &li, UIParaverTraceConfig *pcf,
	ofstream &sortedPCFfile)
{
	sortedPCFfile.open (PCFfile.c_str(), ios_base::out);
	if (!sortedPCFfile.is_open())
	{
		cerr << "Unable to create: " << PCFfile << endl;
		exit (-1);
	}

	DefaultPCFdata (sortedPCFfile);

	/* Sort types 30000100 to 30000199, 80000000 to 80000099 and 60000119.
	   They're used to source code line and source code file information
	   with regular paraver traces obtained with MPItrace */

	sortedPCFfile << "EVENT_TYPE" << endl;
	for (unsigned i = 30000100; i < 30000199; i++)
		if (pcf->getEventType(i) != "")
			sortedPCFfile << "0 " << i << " " << pcf->getEventType(i) << endl;
	for (unsigned i = 80000000; i < 80000099; i++)
		if (pcf->getEventType(i) != "")
			sortedPCFfile << "0 " << i << " " << pcf->getEventType(i) << endl;
	if (pcf->getEventType(60000119) != "")
			sortedPCFfile << "0 " << 60000119 << " " << pcf->getEventType(60000119) << endl;

	sortedPCFfile << "VALUES" << endl;
	sortedPCFfile << "0 " << pcf->getEventValue(30000100, 0) << endl;
	sortedPCFfile << "1 " << pcf->getEventValue(30000100, 1) << endl;
	unsigned i = 2;
	for (list<Information>::iterator it = li.begin(); it != li.end(); i++, it++)
		sortedPCFfile << i << " " << pcf->getEventValue (30000100, (*it).value) << endl;
	sortedPCFfile << endl;
}


void MergePCFs (UIParaverTraceConfig *pcf_original, UIParaverTraceConfig *pcf_new,
	ofstream &pcf_new_file)
{
	vector<unsigned> original_events = pcf_original->getEventTypes();
	vector<unsigned> new_events = pcf_new->getEventTypes();

	for (unsigned i = 0; i < original_events.size(); i++)
	{
		/* Add events that are not already in the new PCF */
		if (find(new_events.begin(), new_events.end(), original_events[i]) == new_events.end())
		{
			pcf_new_file << "EVENT_TYPE" << endl;
			pcf_new_file << "0 " << original_events[i] << " " << pcf_original->getEventType (original_events[i]) << endl;
			vector<unsigned> v = pcf_original->getEventValues (original_events[i]);
			if (v.size() > 0)
			{
				pcf_new_file << "VALUES" << endl;
				for (unsigned j = 0; j < v.size(); j++)
					pcf_new_file << v[j] << " " << pcf_original->getEventValue (original_events[i], v[j]) << endl;
			}
			pcf_new_file << endl;
		}
	}
}

int main (int argc, char *argv[])
{
	string PRVfile;

	if (argc != 2)
	{
		cerr << "Usage:" << endl << argv[0] << " prvfile" << endl;
		return -1;
	}

	PRVfile = argv[1];

	/* Sort line callers (sampling and user function too) event types */
	//UIParaverTraceConfig *pcf = new UIParaverTraceConfig (PRVfile.substr (0, PRVfile.rfind(".prv")) + ".pcf");
	UIParaverTraceConfig *pcf = new UIParaverTraceConfig;
	pcf->parse (PRVfile.substr (0, PRVfile.rfind(".prv")) + ".pcf");
	list<Information> li = SortEvents (pcf);

	/* Generate a new PCF file with the sorted event-types */
	ofstream sortedPCFfile;
	generatePCFfile (PRVfile.substr (0, PRVfile.rfind(".prv")) + ".sorted.pcf", li, pcf, sortedPCFfile);
	//UIParaverTraceConfig *sorted_pcf = new UIParaverTraceConfig (PRVfile.substr (0, PRVfile.rfind(".prv")) + ".sorted.pcf");
	UIParaverTraceConfig *sorted_pcf = new UIParaverTraceConfig;
	sorted_pcf->parse (PRVfile.substr (0, PRVfile.rfind(".prv")) + ".sorted.pcf");

	/* Translate the PRV itself */
	Process *p = new Process (PRVfile, PRVfile.substr (0, PRVfile.rfind(".prv")) + ".sorted.prv", pcf, sorted_pcf);
	p->parseBody();
	delete p;

	/* Feed the new PCF with the remaining events / header */
	MergePCFs (pcf, sorted_pcf, sortedPCFfile);
	sortedPCFfile.close();

	/* Copy the .ROW file */
	ifstream ifs_row ((PRVfile.substr (0, PRVfile.rfind(".prv"))+string(".row")).c_str());
	if (ifs_row.is_open())
	{
		ofstream ofs_row ((PRVfile.substr (0, PRVfile.rfind(".prv"))+string(".sorted.row")).c_str());
		ofs_row << ifs_row.rdbuf();
		ifs_row.close();
		ofs_row.close();
	}
}
