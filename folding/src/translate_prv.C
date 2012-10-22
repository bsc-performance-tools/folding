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

#include "translate_prv.H"

namespace libparaver {

Process::Process (string prvFile, string outname, UIParaverTraceConfig *pcf_original, UIParaverTraceConfig *pcf_reference, bool multievents) : ParaverTrace (prvFile, multievents)
{
	traceout.open (outname.c_str());
	if (!traceout.is_open())
	{
		cout << "Unable to create " << traceout << endl;
		exit (-1);
	}

	this->pcf_original  = pcf_original;
	this->pcf_reference = pcf_reference;
}

Process::~Process ()
{
	traceout.close ();
}

void Process::processComment (string &c)
{
	traceout << "#" << c << endl;
}

void Process::processCommunicator (string &c)
{
	traceout << "c" << c << endl;
}

void Process::processState (struct state_t &s)
{
	traceout << "1:"
		<< s.ObjectID.cpu << ":" << s.ObjectID.ptask << ":" << s.ObjectID.task << ":" << s.ObjectID.thread << ":"
		<< s.Begin_Timestamp << ":" << s.End_Timestamp << ":" << s.State << endl; 
}

void Process::processMultiEvent (struct multievent_t &e)
{
	static bool message_warning = false;

	traceout << "2:"
		<< e.ObjectID.cpu << ":" << e.ObjectID.ptask << ":" << e.ObjectID.task << ":" << e.ObjectID.thread << ":"
		<< e.Timestamp;

	for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
	{
		/* Look at the reference PCF if the type is there. If it is, check which
		   value match with the current one. Otherwise, write the originals. */
		bool translated = false;
		if (pcf_reference->getEventType ((*it).Type) != "")
		{
			string old_value_str = pcf_original->getEventValue ((*it).Type, (*it).Value);
			if (old_value_str != "Not found")
			{
				int new_value = pcf_reference->getEventValue ((*it).Type, old_value_str);
				if (new_value != -1)
				{
					traceout << ":" << (*it).Type << ":" << new_value;
					translated = true;
				}
			}

			if (!translated && !message_warning)
			{
				cout << "WARNING! Type " << (*it).Type << " and value " << (*it).Value << " cannot be find in both original and/or reference PCF files!" << endl;
				message_warning = true;
			}
		}

		if (!translated)
		{
			traceout << ":" << (*it).Type << ":" << (*it).Value;
		}
	}

	traceout << endl;
}

void Process::processEvent (struct singleevent_t &e)
{
	UNREFERENCED(e);
}

void Process::processCommunication (struct comm_t &c)
{
	traceout << "3:" 
		<< c.Send_ObjectID.cpu << ":" << c.Send_ObjectID.ptask << ":" << c.Send_ObjectID.task << ":" << c.Send_ObjectID.thread << ":"
		<< c.Logical_Send << ":" << c.Physical_Send << ":"
		<< c.Recv_ObjectID.cpu << ":" << c.Recv_ObjectID.ptask << ":" << c.Recv_ObjectID.task << ":" << c.Recv_ObjectID.thread << ":"
		<< c.Logical_Recv << ":" << c.Physical_Recv << ":"
		<< c.Size << ":" << c.Tag << endl;
}

} /* namespace libparaver */

