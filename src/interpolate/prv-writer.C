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

#include <stack>

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
	const Instance *in)
{
	assert (type.size() == value.size());

	unsigned ptask = in->getptask();
	unsigned task  = in->gettask();
	unsigned thread = in->getthread();

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
	unsigned long long value, unsigned long long time, const Instance *in)
{
	unsigned ptask = in->getptask();
	unsigned task  = in->gettask();
	unsigned thread = in->getthread();

	/* 2:14:1:14:1:69916704358:40000003:0 */
	traceout << "2:" << task << ":" << ptask << ":" << task << ":" << thread
	  << ":" << time << ":" << type << ":" << value << endl;
}

bool FoldedParaverTrace::DumpCallersInInstance (const Instance *in,
	const InstanceGroup *ig)
{
	set<unsigned long long> set_zero_types;
	vector<unsigned long long> vec_zero_types, vec_zero_values;

	for (const auto & i : ig->getInstances())
		for (const auto & s : i->getSamples())
		{
			unsigned long long ts = in->getStartTime() + 
				(unsigned long long) (s->getNTime() * (double)(in->getDuration()));

			vector<unsigned long long> types;
			vector<unsigned long long> values;
			const map<unsigned, CodeRefTriplet> & ct = s->getCodeTripletsAsConstReference();
			for (auto const & it : ct)
			{
				types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_CALLER_MIN + it.first); /* caller + depth */
				types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_CALLERLINE_MIN + it.first); /* caller line + depth */
				types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_CALLERLINE_AST_MIN + it.first); /* caller line AST + depth */

				values.push_back (it.second.getCaller());
				values.push_back (it.second.getCallerLine());
				values.push_back (it.second.getCallerLineAST());
			}

			DumpParaverLines (types, values, ts, in);

			/* Annotate these types, to emit the corresponding 0s */
			set_zero_types.insert (types.begin(), types.end());
		}

	/* Move types into vec_zero_types */
	vec_zero_types.insert (vec_zero_types.begin(), set_zero_types.begin(),
	  set_zero_types.end());
	vec_zero_values.assign (vec_zero_types.size(), 0);

	DumpParaverLines (vec_zero_types, vec_zero_values,
	  in->getStartTime() + in->getDuration(), in);

	return !set_zero_types.empty();
}

bool FoldedParaverTrace::DumpAddressesInInstance (const Instance *in,
	InstanceGroup *ig)
{
	set<unsigned long long> set_zero_types;
	vector<unsigned long long> vec_zero_types, vec_zero_values;

	for (const auto & s : ig->getAllSamples())
	{
		vector<unsigned long long> types;
		vector<unsigned long long> values;
		unsigned long long ts = in->getStartTime() + 
			(unsigned long long) (s->getNTime() * (double)(in->getDuration()));

		if (s->hasAddressReferenceInfo())
		{
			AddressReferenceType_t art = s->getReferenceType();
			unsigned t = (art == LOAD) ? EXTRAE_SAMPLE_ADDRESS_LD : EXTRAE_SAMPLE_ADDRESS_ST;
			types.push_back (FOLDED_BASE + t);
			types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_ADDRESS_MEM_LEVEL);
			types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_ADDRESS_TLB_LEVEL);
			types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_ADDRESS_REFERENCE_CYCLES);

			values.push_back (s->getAddressReference());
			values.push_back (s->getAddressReference_Mem_Level());
			values.push_back (s->getAddressReference_TLB_Level());
			values.push_back (s->getAddressReference_Cycles_Cost());
		}
		else if (s->hasAddressReference())
		{
			AddressReferenceType_t art = s->getReferenceType();
			unsigned t = (art == LOAD) ? EXTRAE_SAMPLE_ADDRESS_LD : EXTRAE_SAMPLE_ADDRESS_ST;
			types.push_back (FOLDED_BASE + t);
			values.push_back (s->getAddressReference());
		}

		DumpParaverLines (types, values, ts, in);

		/* Annotate these types, to emit the corresponding 0s */
		set_zero_types.insert (types.begin(), types.end());
	}

	/* Move types into vec_zero_types */
	vec_zero_types.insert (vec_zero_types.begin(), set_zero_types.begin(),
	  set_zero_types.end());
	vec_zero_values.assign (vec_zero_types.size(), 0);

	DumpParaverLines (vec_zero_types, vec_zero_values,
	  in->getStartTime() + in->getDuration(), in);

	return !set_zero_types.empty();
}

void FoldedParaverTrace::DumpReverseCorrectedCallersInInstance (
	const Instance *in, const InstanceGroup *ig)
{
	set<unsigned long long> set_zero_types;
	vector<unsigned long long> vec_zero_types, vec_zero_values;

	for (const auto & s : ig->getAllSamples())
	{
		if (!s->getUsableCallstack())
			continue;

#if defined(TIME_BASED_COUNTER)
		string time = common::DefaultTimeUnit;
		string time = "PAPI_TOT_INS";
		unsigned long long ts = in->getStartTime() + 
		  (unsigned long long) (ig->getInterpolatedNTime (time, s) * (double)(in->getDuration()));
#else
		unsigned long long ts = in->getStartTime() + 
			(unsigned long long) (s->getNTime() * (double)(in->getDuration()));
#endif /* TIME_BASED_COUNTER */

		vector<unsigned long long> types;
		vector<unsigned long long> values;

		const map<unsigned, CodeRefTriplet> & ct = s->getCodeTripletsAsConstReference();

		map<unsigned, CodeRefTriplet>::const_reverse_iterator it;
		for (it = ct.crbegin(); it != ct.crend(); it++)
		{
			types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_REVERSE_CALLER_MIN + (*it).first);
			types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_REVERSE_CALLERLINE_MIN + (*it).first);
			types.push_back (FOLDED_BASE + EXTRAE_SAMPLE_REVERSE_CALLERLINE_AST_MIN + (*it).first);

			values.push_back ((*it).second.getCaller());
			values.push_back ((*it).second.getCallerLine());
			values.push_back ((*it).second.getCallerLineAST());
		}

		DumpParaverLines (types, values, ts, in);

		/* Annotate these types, to emit the corresponding 0s */
		set_zero_types.insert (types.begin(), types.end());
	}

	/* Move types into vec_zero_types */
	vec_zero_types.insert (vec_zero_types.begin(), set_zero_types.begin(),
	  set_zero_types.end());
	vec_zero_values.assign (vec_zero_types.size(), 0);

	DumpParaverLines (vec_zero_types, vec_zero_values,
	  in->getStartTime() + in->getDuration(), in);
}

#if 0
void FoldedParaverTrace::DumpReverseMainCallersInInstance (
	const Instance *in, const InstanceGroup *ig, unsigned mainid)
{
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

			const map<unsigned, CodeRefTriplet> & ct = vs[v]->getCodeTripletsAsConstReference();

			/* If top of the callstack is main, process, otherwise ignore */
			if (ct.size() > 0)
			{
				map<unsigned, CodeRefTriplet>::const_reverse_iterator it = ct.crbegin();
				if (((*it).second).getCaller() == mainid)
				{
					vector<unsigned long long> types;
					vector<unsigned long long> values;
					unsigned depth = 0;
					while (it != ct.crend())
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
					DumpParaverLines (types, values, ts, in);

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
	  in->getStartTime() + in->getDuration(), in);
}
#endif


void FoldedParaverTrace::DumpInterpolationData (const Instance *in,
	const InstanceGroup *ig, map<string,unsigned> counterCodes)
{
	vector<unsigned long long> types, values;
	vector<unsigned long long> zero_types, zero_values;
	unsigned steps = 0;

	for (const auto &ir : ig->getInterpolated())
	{
		zero_types.push_back (FOLDED_BASE + counterCodes[ir.first]);
		steps = (ir.second)->getCount();
	}
	zero_values.assign (zero_types.size(), 0);

	DumpParaverLines (zero_types, zero_values, in->getStartTime(), in);

	for (unsigned u = 0; u < steps; u++)
	{
		double proportion = ((double)u) / ((double) steps);
		unsigned long long ts = in->getStartTime() + 
			(unsigned long long) (proportion * (double)(in->getDuration()));

		values.clear();
		types.clear();
		for (const auto &ir : ig->getInterpolated())
		{
			InterpolationResults *irr = ir.second;
			double hwc_values = irr->getInterpolationAt(u) * irr->getAvgCounterValue(); 
			if (u > 0)
			{
				hwc_values -= irr->getInterpolationAt(u-1) * irr->getAvgCounterValue();
				if (hwc_values < 0.)
					hwc_values = 0.;
			}
			values.push_back ((unsigned long long) hwc_values);
			types.push_back (FOLDED_BASE + counterCodes[ir.first]);
		}
		DumpParaverLines (types, values, ts, in);
	}

	DumpParaverLines (zero_types, zero_values, in->getStartTime() + in->getDuration(), in);
}


void FoldedParaverTrace::DumpGroupInfo (const Instance *in,
	unsigned PRVfoldedtype)
{
	vector<unsigned long long> types, values, zero_values;

	types.push_back (FOLDED_INSTANCE_GROUP);
	types.push_back (PRVfoldedtype);
	types.push_back (FOLDED_LAUNCH_TYPE);
	values.push_back (in->getGroup()+1);
	values.push_back (in->getPRVvalue());
	values.push_back (in->getPRVvalue());
	zero_values.assign (types.size(), 0);

	DumpParaverLines (types, values, in->getStartTime(), in);
	DumpParaverLines (types, zero_values, in->getStartTime() + in->getDuration(), in);
}

void FoldedParaverTrace::DumpBreakpoints (const Instance *in,
	const InstanceGroup *ig)
{
	DumpParaverLine (FOLDED_PHASE, 1, in->getStartTime(), in);

	vector<double> breaks = ig->getInterpolationBreakpoints();
	if (breaks.size() >= 2)
	{
		assert (breaks[0]               == 0.0f);
		assert (breaks[breaks.size()-1] == 1.0f);

		for (unsigned p = 1; p < breaks.size()-1; p++)
		{
			unsigned long long delta = (((double) in->getDuration()) * breaks[p]);
			DumpParaverLine (FOLDED_PHASE, p+1, in->getStartTime() + delta, in);
		}
	}

	DumpParaverLine (FOLDED_PHASE, 0, in->getStartTime() + in->getDuration(), in);
}

void FoldedParaverTrace::DumpCallstackProcessed (const Instance *in, 
	const InstanceGroup *ig)
{

	if (!ig->hasPreparedCallstacks())
		return;

	const vector<CallstackProcessor_Result*> events = ig->getPreparedCallstacks();
	assert (events.size() % 2 == 0);

	if (events.size() > 0)
		for (const auto & e : events)
		{
			unsigned long long delta = (((double) in->getDuration()) * e->getNTime());
			DumpParaverLine (FOLDED_CALLER, e->getCodeRef().getCaller(), in->getStartTime() + delta, in);
		}

	/* Dump caller lines within processed routines from callerstime_processSamples */
	const vector<CallstackProcessor_Result*> routines = ig->getPreparedCallstacks();
	vector<CallstackProcessor_Result*>::const_iterator it_ahead = routines.cbegin();
	vector<CallstackProcessor_Result*>::const_iterator it = routines.cbegin();
	stack<CallstackProcessor_Result*> callers;

	if (routines.size() > 0)
		it_ahead++;

	while (it_ahead != routines.cend())
	{
		if ((*it)->getCodeRef().getCaller() > 0)
			callers.push (*it);
		else
			callers.pop();

		if (!callers.empty())
		{
			CallstackProcessor_Result *r = callers.top();	
			unsigned level = r->getLevel();
			double end_time = (*it_ahead)->getNTime();
			double begin_time = (*it)->getNTime();

#if defined(DEBUG)
			unsigned caller = r->getCodeRef().getCaller();
			cout << "Routine " << caller << " at level " << level << " from " << begin_time << " to " << end_time << endl;
#endif

			vector<Sample*> tmp;
			for (const auto & s : ig->getPreparedCallstacks_Process_Samples())
				if (s->getNTime() >= begin_time && s->getNTime() < end_time)
					tmp.push_back (s);

			for (const auto & s : tmp)
			{
				const map<unsigned, CodeRefTriplet> & callers = s->getCodeTripletsAsConstReference();
				if (callers.count (level) > 0)
				{
					CodeRefTriplet crt = callers.at (level);

#if defined(TIME_BASED_COUNTER)
					//string time = common::DefaultTimeUnit;
					string time = "PAPI_TOT_INS";
					unsigned long long ts = in->getStartTime() + 
					  (unsigned long long) (ig->getInterpolatedNTime (time, s) * (double)(in->getDuration()));
#else
					unsigned long long ts = in->getStartTime() +
					  (unsigned long long) (s->getNTime() * (double) (in->getDuration()));
#endif /* TIME_BASED_COUNTER */
	
#if defined(DEBUG) 
					cout << "CRT.getcallerline() = " << crt.getCallerLine() << " @ " << ts << endl;
#endif

					DumpParaverLine (FOLDED_CALLERLINE, crt.getCallerLine(), ts, in);
				}
			}

			tmp.clear();
		}

		it++; it_ahead++;
	}

	DumpParaverLine (FOLDED_CALLERLINE, 0, in->getStartTime() + in->getDuration(), in);

}

} /* namespace libparaver */

