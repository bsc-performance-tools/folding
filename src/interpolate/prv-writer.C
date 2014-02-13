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

#include "prv-writer.H"
#include "prv-types.H"
#include "interpolate.H"

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

void FoldedParaverTrace::DumpParaverLines (const vector<unsigned long long> &type,
	const vector<unsigned long long > &value, unsigned long long time,
	const ObjectSelection *o)
{
	assert (type.size() == value.size());

	unsigned ptask = o->getptask();
	unsigned task  = o->gettask();
	unsigned thread = o->getthread();

	if (type.size() > 0)
	{
		/* 2:14:1:14:1:69916704358:40000003:0 */
		traceout << "2:" << task << ":" << ptask << ":" << task << ":" << thread <<
		  ":" << time;
		for (unsigned i = 0; i < type.size(); i++)
			traceout << ":" << type[i] << ":" << value[i];
		traceout << endl;
	}
}

void FoldedParaverTrace::DumpParaverLine (unsigned long long type,
	unsigned long long value, unsigned long long time, const ObjectSelection *o)
{
	unsigned ptask = o->getptask();
	unsigned task  = o->gettask();
	unsigned thread = o->getthread();

	/* 2:14:1:14:1:69916704358:40000003:0 */
	traceout << "2:" << task << ":" << ptask << ":" << task << ":" << thread
	  << ":" << time << ":" << type << ":" << value << endl;
}

void FoldedParaverTrace::DumpCallersInInstance (const ObjectSelection *o,
	const Instance *in, const InstanceGroup *ig)
{
	assert (!o->anyany());

	vector<Instance*> vInstances = ig->getInstances();
	unsigned numInstances = vInstances.size();

	set<unsigned long long> set_zero_types;
	vector<unsigned long long> vec_zero_types, vec_zero_values;

	for (unsigned u = 0; u < numInstances; u++)
	{
		Instance *i = vInstances[u];

		vector<Sample *> vs = i->getSamples();
		for (unsigned v = 0; v < vs.size(); v++)
		{
			unsigned long long ts = in->getStartTime() + 
				(unsigned long long) (vs[v]->getNTime() * (double)(in->getDuration()));

			vector<unsigned long long> types;
			vector<unsigned long long> values;
			map<unsigned, CodeRefTriplet> ct = vs[v]->getCodeTriplets();
			map<unsigned, CodeRefTriplet>::iterator it;
			for (it = ct.begin(); it != ct.end(); it++)
			{
				types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_CALLER_MIN + (*it).first); /* caller + depth */
				types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_CALLERLINE_MIN + (*it).first); /* caller line + depth */
				types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_CALLERLINE_AST_MIN + (*it).first); /* caller line AST + depth */

				values.push_back ((*it).second.getCaller());
				values.push_back ((*it).second.getCallerLine());
				values.push_back ((*it).second.getCallerLineAST());
			}
			DumpParaverLines (types, values, ts, o);

			/* Annotate these types, to emit the corresponding 0s */
			set_zero_types.insert (types.begin(), types.end());
		}
	}

	/* Move types into vec_zero_types */
	vec_zero_types.insert (vec_zero_types.begin(), set_zero_types.begin(),
	  set_zero_types.end());
	vec_zero_values.assign (vec_zero_types.size(), 0);

	DumpParaverLines (vec_zero_types, vec_zero_values,
	  in->getStartTime() + in->getDuration(), o);
}


void FoldedParaverTrace::DumpReverseMainCallersInInstance (
	const ObjectSelection *o, const Instance *in, const InstanceGroup *ig,
	unsigned mainid)
{
	assert (!o->anyany());

	vector<Instance*> vInstances = ig->getInstances();
	unsigned numInstances = vInstances.size();

	set<unsigned long long> set_zero_types;
	vector<unsigned long long> vec_zero_types, vec_zero_values;

	for (unsigned u = 0; u < numInstances; u++)
	{
		Instance *i = vInstances[u];

		vector<Sample *> vs = i->getSamples();
		for (unsigned v = 0; v < vs.size(); v++)
		{
			unsigned long long ts = in->getStartTime() + 
				(unsigned long long) (vs[v]->getNTime() * (double)(in->getDuration()));

			map<unsigned, CodeRefTriplet> ct = vs[v]->getCodeTriplets();

			/* If top of the callstack is main, process, otherwise ignore */
			if (ct.size() > 0)
			{
				map<unsigned, CodeRefTriplet>::reverse_iterator it = ct.rbegin();
				if (((*it).second).getCaller() == mainid)
				{
					vector<unsigned long long> types;
					vector<unsigned long long> values;
					unsigned depth = 0;
					while (it != ct.rend())
					{
						types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_REVERSE_CALLER_MIN + depth); /* caller + depth */
						types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_REVERSE_CALLERLINE_MIN + depth); /* caller line + depth */
						types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_REVERSE_CALLERLINE_AST_MIN + depth); /* caller line AST + depth */

						values.push_back ((*it).second.getCaller());
						values.push_back ((*it).second.getCallerLine());
						values.push_back ((*it).second.getCallerLineAST());

						it++;
						depth++;
					}
					DumpParaverLines (types, values, ts, o);

					/* Annotate these types, to emit the corresponding 0s */
					set_zero_types.insert (types.begin(), types.end());
				}
			}
		}
	}

	/* Move types into vec_zero_types */
	vec_zero_types.insert (vec_zero_types.begin(), set_zero_types.begin(),
	  set_zero_types.end());
	vec_zero_values.assign (vec_zero_types.size(), 0);

	DumpParaverLines (vec_zero_types, vec_zero_values,
	  in->getStartTime() + in->getDuration(), o);
}


void FoldedParaverTrace::DumpInterpolationData (const ObjectSelection *o,
	const Instance *in, const InstanceGroup *ig, map<string,unsigned> counterCodes)
{
	vector<unsigned long long> types, values;
	vector<unsigned long long> zero_types, zero_values;
	unsigned steps = 0;

	assert (!o->anyany());

	map<string,InterpolationResults*> ir = ig->getInterpolated();
	map<string,InterpolationResults*>::iterator it;
	for (it = ir.begin(); it != ir.end(); it++)
	{
		zero_types.push_back (FOLDED_BASE + counterCodes[(*it).first]);
		steps = ((*it).second)->getCount();
	}
	zero_values.assign (zero_types.size(), 0);

	DumpParaverLines (zero_types, zero_values, in->getStartTime(), o);

	for (unsigned u = 0; u < steps; u++)
	{
		double proportion = ((double)u) / ((double) steps);
		unsigned long long ts = in->getStartTime() + 
			(unsigned long long) (proportion * (double)(in->getDuration()));

		values.clear();
		types.clear();
		for (it = ir.begin(); it != ir.end(); it++)
		{
			InterpolationResults *irr = (*it).second;
			double *hwc_values = irr->getSlopeResultsPtr();
			values.push_back (hwc_values[u]);
			types.push_back (FOLDED_BASE + counterCodes[(*it).first]);
		}
		DumpParaverLines (types, values, ts, o);
	}

	DumpParaverLines (zero_types, zero_values, in->getStartTime() + in->getDuration(), o);
}


void FoldedParaverTrace::DumpGroupInfo (const ObjectSelection *o,
	const Instance *in)
{
	assert (!o->anyany());

	vector<unsigned long long> types, values, zero_values;

	types.push_back (FOLDED_INSTANCE_GROUP);
	types.push_back (FOLDED_TYPE);
	values.push_back (in->getGroup()+1);
	values.push_back (in->getPRVvalue());
	zero_values.assign (types.size(), 0);

	DumpParaverLines (types, values, in->getStartTime(), o);
	DumpParaverLines (types, zero_values, in->getStartTime() + in->getDuration(), o);
}

void FoldedParaverTrace::DumpBreakpoints (const ObjectSelection *o,
	const Instance *in, const InstanceGroup *ig)
{
	assert (!o->anyany());

	DumpParaverLine (FOLDED_PHASE, 1, in->getStartTime(), o);

	vector<double> breaks = ig->getInterpolationBreakpoints();
	if (breaks.size() >= 2)
	{
		assert (breaks[0]               == 0.0f);
		assert (breaks[breaks.size()-1] == 1.0f);

		for (unsigned p = 1; p < breaks.size()-1; p++)
		{
			unsigned long long delta = (((double) in->getDuration()) * breaks[p]);
			DumpParaverLine (FOLDED_PHASE, p+1, in->getStartTime() + delta, o);
		}
	}

	DumpParaverLine (FOLDED_PHASE, 0, in->getStartTime() + in->getDuration(), o);
}

} /* namespace libparaver */

