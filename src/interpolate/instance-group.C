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
#include "pcf-common.H"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "instance-group.H"
#include "interpolation-results.H"
#include "generate-gnuplot.H"

#include <list>
#include <stack>

using namespace std;

InstanceGroup::InstanceGroup (string name, unsigned id, string groupname) :
	regionName (name), numGroup (id), groupName (groupname)
{
	preparedCallstacks = false;
}

void InstanceGroup::add (Instance *i)
{
	Instances.push_back (i);
}

unsigned InstanceGroup::numSamples (void) const
{
	unsigned tmp = 0;
	for (unsigned u = 0; u < Instances.size(); u++)
		tmp += Instances[u]->getNumSamples();

	return tmp;
}

void InstanceGroup::moveToExcluded (Instance *i)
{
	vector<Instance *>::iterator it;

	for (it = Instances.begin(); it != Instances.end(); it++)
		if (i == *it)
		{
			excludedInstances.push_back (i);
			Instances.erase (it);
			break;
		}
}

unsigned InstanceGroup::numExcludedInstances (void) const
{
	return excludedInstances.size();
}

unsigned InstanceGroup::numExcludedSamples (string counter) const
{
	unsigned total = 0;

	for (unsigned i = 0; i < excludedInstances.size(); i++)
	{
		vector<Sample*> vs = excludedInstances[i]->getSamples();
		for (unsigned u = 0; u < vs.size(); u++)
			if (vs[u]->hasCounter (counter))
				total++;
	}

	return total;
}

unsigned InstanceGroup::numExcludedSamples (void) const
{
	unsigned tmp = 0;
	for (unsigned u = 0; u < excludedInstances.size(); u++)
		tmp += excludedInstances[u]->getNumSamples();

	return tmp;
}

unsigned long long InstanceGroup::mean (const string &counter) const
{
	if (Instances.size() == 0)
		return 0;

	if (counter == common::DefaultTimeUnit)
		return mean();

	unsigned long long tmp = 0;
	unsigned long long count = 0;
	for (unsigned u = 0; u < Instances.size(); u++)
		if (Instances[u]->hasCounter (counter))
		{
			tmp += Instances[u]->getTotalCounterValue (counter);
			count++;
		}

	if (count > 0)
		return tmp / count;
	else
		return 0;
}

unsigned long long InstanceGroup::mean (void) const
{
	if (Instances.size() == 0)
		return 0;

	unsigned long long tmp = 0;
	for (unsigned u = 0; u < Instances.size(); u++)
		tmp += Instances[u]->getDuration();

	return tmp / Instances.size();
}

unsigned long long InstanceGroup::median (void) const
{
	if (Instances.size() == 0)
		return 0;

	vector<unsigned long long> durations;
	for (unsigned u = 0; u < Instances.size(); u++)
		durations.push_back (Instances[u]->getDuration());

	sort (durations.begin(), durations.end());

	return durations[Instances.size() / 2];
}

double InstanceGroup::stdev (void) const
{
	if (Instances.size() <= 1)
		return 0;

	double tmp = 0;
	double _mean;

	_mean = this->mean();
	for (unsigned u = 0; u < Instances.size(); u++)
	{
		double d = Instances[u]->getDuration();
		tmp += ( (d-_mean) * (d-_mean) );
	}

	return sqrt ( tmp / (Instances.size() -1) );
}

unsigned long long InstanceGroup::MAD (void) const
{
	if (Instances.size() == 0)
		return 0;

	vector<unsigned long long> absmediandiff;
	unsigned long long _median;

	_median = this->median();
	for (unsigned u = 0; u < Instances.size(); u++)
		absmediandiff.push_back (llabs (Instances[u]->getDuration() - _median));

	sort (absmediandiff.begin(), absmediandiff.end());

	return absmediandiff[Instances.size() / 2];
}

void InstanceGroup::removePreviousData (ObjectSelection *os, const string &prefix)
{
	string fname = prefix + "." + os->toString(false, "any") +
	  ".dump.csv";
	string fintname = prefix + "." + os->toString(false, "any") +
	  ".interpolate.csv";
	string fslname = prefix + "." + os->toString(false, "any") +
	  ".slope.csv";

	if (common::existsFile (fname))
		if (unlink (fname.c_str()) != 0)
			cerr << "Can't remove previous file '" << fname << "'" << endl;
	if (common::existsFile (fintname))
		if (unlink (fintname.c_str()) != 0)
			cerr << "Can't remove previous file '" << fintname << "'" << endl;
	if (common::existsFile (fslname))
		if (unlink (fslname.c_str()) != 0)
			cerr << "Can't remove previous file '" << fslname << "'" << endl;
}

void InstanceGroup::dumpInterpolatedData (ObjectSelection *os, 
	const string & prefix, const vector<Model*> & models)
{
	string fintname = prefix + "." + os->toString(false, "any") +
	  ".interpolate.csv";
	string fslname = prefix + "." + os->toString(false, "any") +
	  ".slope.csv";

	if (Instances.size() > 0 && interpolated.size() > 0)
	{
		unsigned nsteps = (*(interpolated.begin())).second->getCount();
		assert (nsteps > 0);
		double d_steps = (double) nsteps;

		ofstream int_data, sl_data;
		int_data.open (fintname.c_str(), std::ofstream::app);
		sl_data.open (fslname.c_str(), std::ofstream::app);

		assert (int_data.is_open());
		assert (sl_data.is_open());

		bool instructionCounterPresent = false;
		string instructionsCounter;

		map<string, InterpolationResults*>::iterator it;
		for (it = interpolated.begin(); it != interpolated.end(); it++)
		{
			// If this is a MIPS counter, annotate it to calculate counters vs inst ratios
			if (common::isMIPS((*it).first) && !instructionCounterPresent)
			{
				instructionsCounter = (*it).first;
				instructionCounterPresent = true;
			}

			InterpolationResults *ir = (*it).second;

			// This should always happen, but it's good to check
			assert (ir->getCount() == nsteps);

			string counter = (*it).first;
			for (unsigned u = 0; u < nsteps; u++)
			{
				double d_j = (double) u;
				int_data << regionName << ";" << numGroup << ";" << counter << 
				  ";" << d_j / d_steps << ";" << ir->getInterpolationAt(u) << endl;
			}
			for (unsigned u = 1; u < nsteps; u++)
			{
				double d_j = (double) u;
				sl_data << regionName << ";" << numGroup << ";" << counter <<
				  ";" << d_j / d_steps << ";" << ir->getSlopeAt(u) << endl;
			}
		}

		// Process all counters and calculate the ratio against instruction counter
		if (instructionCounterPresent)
		{
			InterpolationResults *ir_instruction = interpolated[instructionsCounter];
			double *data_s_i = ir_instruction->getSlopeResultsPtr();
	
			for (it = interpolated.begin(); it != interpolated.end(); it++)
			{
				// Skip counters related to MIPS
				if (common::isMIPS((*it).first))
					continue;
	
				InterpolationResults *ir = (*it).second;
	
				// This should always happen, but it's good to check
				assert (nsteps == ir->getCount());
	
				string counter = (*it).first;
				double *data_s = ir->getSlopeResultsPtr();
				for (unsigned u = 1; u < nsteps; u++)
				{
					double d_j = (double) u;
					sl_data << regionName << ";" << numGroup << ";" << counter <<
					  "_per_ins;" << d_j / d_steps << ";" << data_s[u]/data_s_i[u] << endl;
				}
			}
		}

		// Process models, generate their data
		// Unfortunately, we need to generate one csv file per component of
		// the model
		for (unsigned m = 0; m < models.size(); m++)
		{
			Model *model = models[m];
			vector<ComponentModel*> vcm = model->getComponents();
			for (unsigned cm = 0; cm < vcm.size(); cm++)
			{
				const ComponentNode * cn = vcm[cm]->getComponentNode();
				for (unsigned u = 1; u < nsteps; u++)
				{
					double d_j = (double) u;
					sl_data << regionName << ";" << numGroup << ";" <<
					  model->getName() << "_" << vcm[cm]->getName() << ";" <<
					  d_j / d_steps << ";" << cn->evaluate (interpolated, u) << endl;
				}
			}
		}

		int_data.close ();
		sl_data.close ();
	}
}

static bool compare_SampleTimings (Sample *s1, Sample *s2)
{
	return s1->getNTime() < s2->getNTime();
	//return s1->getNCounterValue("PAPI_TOT_INS") < s2->getNCounterValue("PAPI_TOT_INS");
}

void InstanceGroup::dumpData (ObjectSelection *os, const string & prefix,
	UIParaverTraceConfig *pcf)
{
	string fname = prefix + "." + os->toString(false, "any") +
	  ".dump.csv";

	ofstream odata;
	odata.open (fname.c_str(), std::ofstream::app);

	if (Instances.size() > 0)
	{
		vector<Sample*> timingsamples;

		/* Emit hwc and addresses for both used & unused sets */
		for (auto & instance : used)
		{
			string counter = instance.first;
			for (auto s : instance.second)
			{
				if (s->hasCounter(counter))
					odata << "u" << ";" << regionName << ";" << numGroup << ";"
					  << s->getNTime() << ";" << counter << ";"
					  << s->getNCounterValue(counter) << endl;
				if (s->hasAddressReference())
				{
					odata << "a" << ";" << regionName << ";" << numGroup << ";"
					  << s->getNTime() << ";" << s->getAddressReference() << ";"
					  << s->getAddressReference_Mem_Level() << ";"
					  << s->getAddressReference_Cycles_Cost() << endl;
				}
				if (s->hasCodeRefTripletSize())
					timingsamples.push_back (s);
			}
		}
		for (auto & instance : unused)
		{
			string counter = instance.first;
			for (auto s : instance.second)
			{
				if (s->hasCounter(counter))
					odata << "un" << ";" << regionName << ";" << numGroup << ";"
					  << s->getNTime() << ";" << counter << ";"
					  << s->getNCounterValue(counter) << endl;
				if (s->hasAddressReference())
				{
					odata << "a" << ";" << regionName << ";" << numGroup << ";"
					  << s->getNTime() << ";" << s->getAddressReference() << ";"
					  << s->getAddressReference_Mem_Level() << ";"
					  << s->getAddressReference_Cycles_Cost() << endl;
				}
				if (s->hasCodeRefTripletSize())
					timingsamples.push_back (s);
			}
		}

		/* Dump caller lines within processed routines from callerstime_processSamples */
		{
			const vector<CallstackProcessor_Result*> routines = this->getPreparedCallstacks();
			vector<CallstackProcessor_Result*>::const_iterator it_ahead = routines.cbegin();
			vector<CallstackProcessor_Result*>::const_iterator it = routines.cbegin();
			stack<CallstackProcessor_Result*> callers;

			it_ahead++;

			while (it_ahead != routines.cend())
			{
				if ((*it)->getCaller() > 0)
					callers.push (*it);
				else
					callers.pop();

				CallstackProcessor_Result *r = callers.top();	
				unsigned level = r->getLevel();
				unsigned caller = r->getCaller();
				double end_time = (*it_ahead)->getNTime();
				double begin_time = (*it)->getNTime();

#if defined(DEBUG)
				cout << "Routine " << caller << " at level " << level << " from " << begin_time << " to " << end_time << endl;
#endif

				vector<Sample*> tmp;
				for (auto s : callerstime_processSamples)
					if (s->getNTime() >= begin_time && s->getNTime() < end_time)
						tmp.push_back (s);

				for (const auto s : tmp)
				{
					const map<unsigned, CodeRefTriplet> & callers = s->getCodeTripletsAsConstReference();
					assert (callers.count (level) > 0);
					CodeRefTriplet crt = callers.at (level);

#if defined(DEBUG)
					cout << "CRT.getcallerline() = " << crt.getCallerLine() << endl;
#endif

					unsigned codeline;
					string file;
					pcfcommon::lookForCallerLineInfo (pcf, crt.getCallerLine(), file,
					  codeline);

					if (codeline > 0)
						odata << "cl" << ";" << regionName << ";" << numGroup << ";"
						  << s->getNTime() << ";" << codeline << ";" << crt.getCaller()
						  << endl;
				}

				tmp.clear();

				it++; it_ahead++;
			}

		}

	}

	if (excludedInstances.size() > 0)
	{
		for (auto & i : excludedInstances)
			for (auto & s : i->getSamples())
				for (auto & c : s->getNCounterValue())
					odata << "e" << ";" << regionName << ";" << numGroup << ";"
					  << s->getNTime() << ";" << c.first << ";" << c.second << endl;
	}

	odata.close ();
}

void InstanceGroup::gnuplot (const ObjectSelection *os, const string & prefix,
	const vector<Model*> & models, const string &TimeUnit,
	vector<VariableInfo*> & variables, const map<unsigned,string> & hParaverIdRoutine)
{
	bool has_instruction_counter = false;
	map<string, InterpolationResults*>::iterator it;

	/* Dump single plots first */
	for (it = interpolated.begin(); it != interpolated.end(); it++)
	{
		has_instruction_counter |= common::isMIPS((*it).first);
		gnuplotGenerator::gnuplot_single (this, os, prefix, (*it).first,
		  (*it).second, TimeUnit, hParaverIdRoutine);
	}

	/* If has instruction counter, generate this in addition to .slopes */
	string ofile;
	if (has_instruction_counter)
		ofile = gnuplotGenerator::gnuplot_slopes (this, os, prefix,
		  true, TimeUnit, hParaverIdRoutine);
	else
		ofile = gnuplotGenerator::gnuplot_slopes (this, os, prefix, false,
		  TimeUnit, hParaverIdRoutine);
	cout << "Summary plot for region " << regionName << " ("
	  << ofile << ")"  << endl;

	if (hasAddresses())
	{
		string name_addresses = gnuplotGenerator::gnuplot_addresses (this, os,
		  prefix, TimeUnit, variables, hParaverIdRoutine);
		if (name_addresses.length() > 0)
			cout << "Summary plot for region " << regionName << " ("
			  << name_addresses << ")" << endl;
		name_addresses = gnuplotGenerator::gnuplot_addresses_cost (this, os,
		  prefix, TimeUnit, variables, hParaverIdRoutine);
		if (name_addresses.length() > 0)
			cout << "Summary plot for region " << regionName << " ("
			  << name_addresses << ")" << endl;
	}

	for (unsigned m = 0; m < models.size(); m++)
	{
		string tmp = gnuplotGenerator::gnuplot_model (this, os, prefix,
		  models[m], TimeUnit, hParaverIdRoutine);
		cout << "Plot model " << models[m]->getTitleName() << " for region "
		  << regionName << " (" << tmp << ")" << endl;
	}
}

string InstanceGroup::python (void)
{
	double MIPS = 0;

	map<string, InterpolationResults*>::iterator it;
	for (it = interpolated.begin(); it != interpolated.end(); it++)
		if (common::isMIPS((*it).first))
		{
			double avginst = (*it).second->getAvgCounterValue();
			double avgtime = mean();
			MIPS = 1000.f * ( avginst / avgtime );
			break;
		}

	return common::removeSpaces(groupName) + 
	  "-" + common::convertDouble (mean() / 1000000.f, 3) +
	  "-" + common::convertDouble (MIPS, 3) +
	  "-" + common::convertInt (Instances.size()) +
	  "-" + common::convertInt (Breakpoints.size() > 0 ? Breakpoints.size()-1 : 0);
}

void InstanceGroup::setSamples (map<string, vector<Sample*> > &used,
	map<string, vector<Sample*> > &unused)
{
	this->used = used;
	this->unused = unused;

	allsamples.clear();

	// Add used samples to all samples
	for (auto instance : used)
		for (auto s : instance.second)
			allsamples.insert (s);

	// Add unused samples to all samples
	for (auto instance : unused)
		for (auto s : instance.second)
			allsamples.insert (s);
}

bool InstanceGroup::hasAddresses (void) const
{
	for (auto s : allsamples)
		if (s->hasAddressReference())
			return true;
	return false;
}

static bool compare_pair_caller_quantity (
	const pair<unsigned, unsigned> & first,
	const pair<unsigned, unsigned> & second)
{ return first.second > second.second;}

// #define DEBUG

void InstanceGroup::prepareCallstacks (CallstackProcessor *processor)
{
	/* Keep samples that contain callers within 5 most frequent */
	/* if we want to cover up to a certain %, we need to modify this 5 */

	if (preparedCallstacks)
		return;

	vector<Sample*> workSamples, processSamples, allSamples;

	for (const auto & instance : used)
		for (auto s : instance.second)
			if (s->hasCodeRefTripletSize())
				workSamples.push_back (s);
	for (const auto & instance : unused)
		for (auto s : instance.second)
			if (s->hasCodeRefTripletSize())
				workSamples.push_back (s);

	allSamples = workSamples;

	sort (workSamples.begin(), workSamples.end(), ::compare_SampleTimings);

	unsigned tenpct = workSamples.size() / 10;
	unsigned level = 0;
	while (level < 1 && workSamples.size() > tenpct)
	{
		vector<Sample*> selectedSamples;

		/* Compute each caller's frequency */
		map<unsigned, unsigned> CallerQuantity;
		map<unsigned, unsigned> CallerMaxDepth;
		map<unsigned, Sample*> CallerMaxDepth_sample;
		map<unsigned, unsigned> CallerMinDepth;
		map<unsigned, Sample*> CallerMinDepth_sample;
		bool hasMaxMinCaller = false;
		unsigned MaxCaller = 0;

		for (auto s : workSamples)
		{
			const Callstack_CodeRefTriplet & current_ct = s->getCallstackCodeRefTriplet();
			const map<unsigned, CodeRefTriplet> triplets = current_ct.getAsConstReferenceMap();

			for (auto const triplet : triplets)
			{
				unsigned caller = triplet.second.getCaller();
				if (CallerQuantity.count(caller) == 0)
				{
					CallerQuantity[caller] = 1;
					CallerMaxDepth[caller] = triplet.first;
					CallerMinDepth[caller] = triplet.first;
					CallerMaxDepth_sample[caller] = s;
					CallerMinDepth_sample[caller] = s;
				}
				else
				{
					CallerQuantity[caller]++;
					if (triplet.first > CallerMaxDepth[caller])
					{
						CallerMaxDepth[caller] = triplet.first;
						CallerMaxDepth_sample[caller] = s;
					}
					if (triplet.first < CallerMinDepth[caller])
					{
						CallerMinDepth[caller] = triplet.first;
						CallerMinDepth_sample[caller] = s;
					}
				}

				if (hasMaxMinCaller)
				{
					if (CallerQuantity[caller] > CallerQuantity[MaxCaller])
						MaxCaller = caller;
				}
				else
					MaxCaller = caller;
			}
		}

		/* Sort vector according to their caller frequency */
		vector < pair < unsigned, unsigned> > vCallerQuantity (
		  CallerQuantity.begin(), CallerQuantity.end());
		std:sort (vCallerQuantity.begin(), vCallerQuantity.end(), ::compare_pair_caller_quantity);

		/* select samples directly-related with top level */
		unsigned caller_at_top = vCallerQuantity[0].first;
		vector<Sample*>::iterator it = workSamples.begin();
		while (it != workSamples.end())
		{
			if ((*it)->hasCaller (caller_at_top))
			{
				(*it)->setUsableCallstack (true);
				selectedSamples.push_back (*it);
				processSamples.push_back (*it);
				it = workSamples.erase (it);
			}
			else
				it++;
		}

#if defined(DEBUG)
		cout << endl;
		cout << "Level " << level << " for caller " << caller_at_top
		  << " contains " << selectedSamples.size() << " out of " 
		  << allSamples.size() << " (" << ((double)selectedSamples.size()/(double)allSamples.size()) << ")" << endl;
		cout << "Caller " << caller_at_top << " max depth = " << CallerMaxDepth[caller_at_top] << endl;
		CallerMaxDepth_sample[caller_at_top]->show(false);
		cout << "Caller " << caller_at_top << " min depth = " << CallerMinDepth[caller_at_top] << endl;
		CallerMinDepth_sample[caller_at_top]->show(false);
		cout << endl;
#endif

		for (auto s: selectedSamples)
		{
#if defined(DEBUG)
			s->show(false);
#endif
			s->addCallstackBubbles (CallerMaxDepth[caller_at_top] - s->getCallerLevel (caller_at_top));
#if defined(DEBUG)
			s->show(false);
#endif
		}

#if defined(DEBUG)
		cout << endl;
		cout << "Correcting bottom part with Caller " << caller_at_top << " min depth = " << CallerMinDepth[caller_at_top] << endl;
		CallerMinDepth_sample[caller_at_top]->show(false);
		cout << endl;
#endif

#if 1
		for (auto s: selectedSamples)
		{
#if defined(DEBUG)
			s->show(false);
#endif
			s->copyBottomStack (CallerMinDepth_sample[caller_at_top]);
#if defined(DEBUG)
			s->show(false);
#endif
		}

#endif

#if 1

#if defined(DEBUG)
		cout << "Matching remaining samples" << endl;
#endif

		unsigned counter = 0;

		/* try to match remaining samples */
		it = workSamples.begin();
		while (it != workSamples.end())
		{
			counter++;

			const Callstack_CodeRefTriplet & work_ct = (*it)->getCallstackCodeRefTriplet();

			int distance = 0;
			bool match = false;
			for (auto s : selectedSamples)
			{
				const Callstack_CodeRefTriplet & s_ct = s->getCallstackCodeRefTriplet();

				distance = work_ct.prefix_match (s_ct, match);
				if (match)
				{
#if defined(DEBUG)
					cout << "MATCH distance = " << distance << endl;
					cout << "S1)"; (*it)->show(false); // work_ct.show (false);
					cout << "S2)"; s->show(false); // s_ct.show (false);
#endif
					if (distance > 0)
					{
						(*it)->addCallstackBubbles (distance);
#if defined(DEBUG)
						(*it)->show();
#endif
					}
					break;
				}

				distance = s_ct.prefix_match (work_ct, match);
				if (match)
				{
					if (distance <= 0)
					{
#if defined(DEBUG)
						cout << "REV_MATCH_A distance = " << distance << endl;
						cout << "S1)"; (*it)->show(false); // work_ct.show (false);
						cout << "S2)"; s->show(false); // s_ct.show (false);
#endif
						(*it)->addCallstackBubbles ((unsigned)(-distance));
						(*it)->copyBottomStack (s);
#if defined(DEBUG)
						(*it)->show(false);
#endif
					}
					else
					{
						match = false;

#if 0
#if defined(DEBUG)
						cout << "REV_MATCH_B distance = " << distance << endl;
						cout << "S1)"; work_ct.show (false);
						cout << "S2)"; s_ct.show (false);

						cout << "CORRECTION PHASE! (distance = " << distance << ")" << endl;
#endif
						for (auto ss : selectedSamples)
						{
#if defined(DEBUG)
							ss->show();
#endif
							ss->addCallstackBubbles (distance);								
#if defined(DEBUG)
							ss->show();
#endif
						}
#endif
					}
					break;
				}

			}

			if (match)
			{
				(*it)->setUsableCallstack (true);

#if defined(DEBUG)
				cout << "RECORRECTING match" << endl;
#endif

				for (auto s: selectedSamples)
				{
#if defined(DEBUG)
					s->show(false);
#endif
					s->copyBottomStack (*it);
#if defined(DEBUG)
					s->show(false);
#endif
				}

				selectedSamples.push_back (*it);
				processSamples.push_back (*it);
				it = workSamples.erase (it);
			}
			else
				it++;
		}

#endif

#if 0
		unsigned max_depth = 0;
		for (auto s : selectedSamples)
			max_depth = MAX(max_depth, s->getMaxCallerLevel());
		for (auto s : selectedSamples)
			s->setCallstackMaxDepth (max_depth);
#endif

#if defined(DEBUG)
		cout << "postLevel " << level << " for caller " << caller_at_top
		  << " contains " << selectedSamples.size() << " out of " 
		  << allSamples.size() << " (" << ((double)selectedSamples.size()/(double)allSamples.size()) << ")" << endl;
#endif

		/* clear containers for the next round */
		selectedSamples.clear();
		CallerQuantity.clear();
		CallerMaxDepth.clear();
		level++;
	}

	preparedCallstacks = true;

	sort (processSamples.begin(), processSamples.end(), ::compare_SampleTimings);

	/* Do process the samples to locate user functions */
	callerstime_Results = processor->processSamples (processSamples);

	/* Store a copy to dump it later */
	callerstime_processSamples = processSamples;
}

double InstanceGroup::getInterpolatedNTime (const string &counter, Sample *s) const
{
	if (counter != common::DefaultTimeUnit)
	{
		assert (interpolated.count(counter) > 0);
		assert (s->hasCounter(counter));

		double res = 0.;

		InterpolationResults * ir = interpolated.at(counter);
		double valuetofind = s->getNCounterValue (counter);
		unsigned n = ir->getCount();
		double *ptr = ir->getInterpolationResultsPtr();
			
		for (unsigned u = 0; u < n; u++)
		{
			if (ptr[u] > valuetofind)
			{
				/* Approximate between two points */
				if (u > 0)
				{
					double maxctr = ptr[u];
					double minctr = ptr[u-1];
					double pos = (maxctr - valuetofind)/(maxctr - minctr);
					double tstep = ((double)1/(double)n);
					res = ((double)(u-1)/(double)(n)) + tstep * pos;
				}
				else
					res = 0.;
				break;
			}
			else if (ptr[u] == valuetofind)
			{
				res = ((double)(u)/(double)(n));
				break;
			}
		}
		return res;
	}
	else
		return s->getNTime();
}

