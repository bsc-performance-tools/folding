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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/interpolate.C $
 | 
 | @last_commit: $Date: 2013-05-24 16:08:28 +0200 (dv, 24 mai 2013) $
 | @version:     $Revision: 1764 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id: interpolate.C 1764 2013-05-24 14:08:28Z harald $";

#include "common.H"

#include "prv-writer.H"

namespace libparaver {

FoldedParaverTrace::FoldedParaverTrace (string &traceout, string prvFile, bool multievents) : ParaverTrace (prvFile, multievents)
{
	this->traceout.open (traceout.c_str());
}

void FoldedParaverTrace::processComment (string &c)
{
	traceout << "#" << c << endl;
}

void FoldedParaverTrace::processCommunicator (string &c)
{
	traceout << "c" << c << endl;
}

void FoldedParaverTrace::processState (struct state_t &s)
{
	UNREFERENCED(s);
} 

void FoldedParaverTrace::processMultiEvent (struct multievent_t &e)
{
	UNREFERENCED(e);
}

void FoldedParaverTrace::processEvent (struct singleevent_t &e)
{
	UNREFERENCED(e);
}

void FoldedParaverTrace::processCommunication (struct comm_t &c)
{
	UNREFERENCED(c);
}

void FoldedParaverTrace::DumpStartingParaverLine (void)
{
	time_t now = time(0);

	traceout << "# Folding done by " << getenv("USER") << " at " << ctime(&now);
}

void FoldedParaverTrace::DumpParaverLines (vector<unsigned long long> &type,
	vector<unsigned long long > &value, unsigned long long time, unsigned ptask,
	unsigned task, unsigned thread)
{
	assert (type.size() == value.size());

	if (type.size() > 0)
	{
		/* 2:14:1:14:1:69916704358:40000003:0 */
		traceout << "2:" << task << ":1:" << task << ":" << thread << ":" << time;
		for (unsigned i = 0; i < type.size(); i++)
			traceout << ":" << type[i] << ":" << value[i];
		traceout << endl;
	}
}

void FoldedParaverTrace::DumpParaverLine (unsigned long long type,
	unsigned long long value, unsigned long long time, unsigned ptask,
	unsigned task, unsigned thread)
{
	/* 2:14:1:14:1:69916704358:40000003:0 */
	traceout << "2:" << task << ":" << ptask << ":" << task << ":" << thread
	  << ":" << time << ":" << type << ":" << value << endl;
}

void FoldedParaverTrace::DumpCallersInInstance (ObjectSelection *o, Instance *in,
	InstanceGroup *ig)
{
	vector<Instance*> vInstances = ig->getInstances();
	unsigned numInstances = vInstances.size();
	vector<unsigned long long> zero_types;
	vector<unsigned long long> zero_values;

	assert (!o->anyany());

	if (numInstances > 0)
	{
		Instance *i = vInstances[0];

		map<unsigned, CodeRefTriplet>::iterator it = i->Samples[0]->CodeTriplet.begin();
		for (; it != i->Samples[0]->CodeTriplet.end(); it++)
		{
			zero_types.push_back (630000000 + (*it).first); /* caller + depth */
			zero_types.push_back (630000100 + (*it).first); /* caller line + depth */
			zero_types.push_back (630000200 + (*it).first); /* caller line AST + depth */

			zero_values.push_back (0);
			zero_values.push_back (0);
			zero_values.push_back (0);
		}
		DumpParaverLines (zero_types, zero_values, in->startTime, o->ptask, o->task, o->thread);
	}

	for (unsigned u = 0; u < numInstances; u++)
	{
		Instance *i = vInstances[u];

		for (unsigned v = 0; v < i->Samples.size(); v++)
		{
			unsigned long long ts = in->startTime + 
				(unsigned long long) (i->Samples[v]->nTime * (double)(in->duration));

			vector<unsigned long long> types;
			vector<unsigned long long> values;
			map<unsigned, CodeRefTriplet>::iterator it = i->Samples[v]->CodeTriplet.begin();
			for (; it != i->Samples[v]->CodeTriplet.end(); it++)
			{
				types.push_back (630000000 + (*it).first); /* caller + depth */
				types.push_back (630000100 + (*it).first); /* caller line + depth */
				types.push_back (630000200 + (*it).first); /* caller line AST + depth */

				values.push_back ((*it).second.Caller);
				values.push_back ((*it).second.CallerLine);
				values.push_back ((*it).second.CallerLineAST);
			}
			DumpParaverLines (types, values, ts, o->ptask, o->task, o->thread);
		}
	}

	if (numInstances > 0)
		DumpParaverLines (zero_types, zero_values, in->startTime + in->duration, o->ptask, o->task, o->thread);
}

void FoldedParaverTrace::DumpInterpolationData (ObjectSelection *o, Instance *in,
	InstanceGroup *ig, map<string,unsigned> counterCodes)
{
	vector<unsigned long long> values;
	vector<unsigned long long> types;
	vector<unsigned long long> zero_values;
	unsigned steps;

	assert (!o->anyany());

	map<string,InterpolationResults*> ir = ig->getInterpolated();
	map<string,InterpolationResults*>::iterator it;
	for (it = ir.begin(); it != ir.end(); it++)
	{
		types.push_back (600000000 + counterCodes[(*it).first]);
		zero_values.push_back (0);
		steps = ((*it).second)->getCount();
	}

	DumpParaverLines (types, zero_values, in->startTime, o->ptask, o->task, o->thread);

	for (unsigned u = 0; u < steps; u++)
	{
		double proportion = ((double)u) / ((double) steps);
		unsigned long long ts = in->startTime + 
			(unsigned long long) (proportion * (double)(in->duration));

		values.clear();
		for (it = ir.begin(); it != ir.end(); it++)
		{
			InterpolationResults *irr = (*it).second;
			double *hwc_values = irr->getSlopeResultsPtr();
			values.push_back (hwc_values[u]);
			DumpParaverLines (types, values, ts, o->ptask, o->task, o->thread);
		}
	}

	DumpParaverLines (types, zero_values, in->startTime + in->duration, o->ptask, o->task, o->thread);
}




} /* namespace libparaver */

