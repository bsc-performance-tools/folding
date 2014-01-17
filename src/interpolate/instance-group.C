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

using namespace std;

InstanceGroup::InstanceGroup (string name, unsigned id, string groupname) :
	regionName (name), numGroup (id), groupName (groupname)
{
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
			double *data_i = ir->getInterpolationResultsPtr();
			double *data_s = ir->getSlopeResultsPtr();
			for (unsigned u = 0; u < nsteps; u++)
			{
				double d_j = (double) u;
				int_data << regionName << ";" << numGroup << ";" << counter << 
				  ";" << d_j / d_steps << ";" << data_i[u] << endl;
			}
			for (unsigned u = 1; u < nsteps; u++)
			{
				double d_j = (double) u;
				sl_data << regionName << ";" << numGroup << ";" << counter <<
				  ";" << d_j / d_steps << ";" << data_s[u] << endl;
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

void InstanceGroup::dumpData (ObjectSelection *os, const string & prefix)
{
	string fname = prefix + "." + os->toString(false, "any") +
	  ".dump.csv";

	ofstream odata;
	odata.open (fname.c_str(), std::ofstream::app);

	if (Instances.size() > 0)
	{
		/* Emit hwc for both used & unused sets */
		map<string, vector<Sample*> >::iterator it;
		for (it = used.begin(); it != used.end(); it++)
		{
			string counter = (*it).first;
			vector<Sample*> usedSamples = (*it).second;
			for (unsigned u = 0; u < usedSamples.size(); u++)
			{
				Sample *s = usedSamples[u];
				if (s->hasCounter(counter))
					odata << "u" << ";" << regionName << ";" << numGroup << ";"
					  << s->getNTime() << ";" << counter << ";"
					  << s->getNCounterValue(counter) << endl;
			}
		}
		for (it = unused.begin(); it != unused.end(); it++)
		{
			string counter = (*it).first;
			vector<Sample*> unusedSamples = (*it).second;
			for (unsigned u = 0; u < unusedSamples.size(); u++)
			{
				Sample *s = unusedSamples[u];
				if (s->hasCounter(counter))
					odata << "un" << ";" << regionName << ";" << numGroup << ";"
					  << s->getNTime() << ";" << counter << ";"
					  << s->getNCounterValue(counter) << endl;
			}
		}
		/* Emit addresses for those samples that have them */
		set<Sample *>::iterator sit;
		for (sit = allsamples.begin(); sit != allsamples.end(); sit++)
		{
			Sample *s = *sit;
			if (s->hasAddressReference())
				odata << "a" << ";" << regionName << ";" << numGroup << ";"
				  << s->getNTime() << ";" << s->getAddressReference() << ";"
				  << s->getAddressReference_Mem_Level() << endl;
		}
	}

	if (excludedInstances.size() > 0)
	{
		for (unsigned i = 0; i < excludedInstances.size(); i++)
		{
			vector<Sample*> vs = excludedInstances[i]->getSamples();
			for (unsigned s = 0; s < vs.size(); s++)
			{
				double time = vs[s]->getNTime();
				map<string, double> counters = vs[s]->getNCounterValue();
				map<string, double>::iterator it;
				for (it = counters.begin() ; it != counters.end(); it++)
					odata << "e" << ";" << regionName << ";" << numGroup << ";"
					  << time << ";" << (*it).first << ";" << (*it).second << endl;
			}
		}
	}

	odata.close ();
}

void InstanceGroup::gnuplot (const ObjectSelection *os, const string & prefix,
	const vector<Model*> & models, const string &TimeUnit,
	vector<VariableInfo*> & variables)
{
	bool has_instruction_counter = false;
	map<string, InterpolationResults*>::iterator it;

	/* Dump single plots first */
	for (it = interpolated.begin(); it != interpolated.end(); it++)
	{
		has_instruction_counter |= common::isMIPS((*it).first);
		gnuplotGenerator::gnuplot_single (this, os, prefix, (*it).first,
		  (*it).second, TimeUnit);
	}

	/* If has instruction counter, generate this in addition to .slopes */
	string name_slopes, name_inst_ctr;
	if (has_instruction_counter)
		name_inst_ctr = gnuplotGenerator::gnuplot_slopes (this, os, prefix,
		  true, TimeUnit);
	name_slopes = gnuplotGenerator::gnuplot_slopes (this, os, prefix, false,
	  TimeUnit);

	/* If has instruction counter, let the new plot be the summary, otherwise
	   let the .slopes be the summary */
	if (has_instruction_counter && name_inst_ctr.length() > 0)
		cout << "Summary plot for region " << regionName << " ("
		  << name_inst_ctr << ")" << endl;
	if (name_slopes.length() > 0)
		cout << "Summary plot for region " << regionName << " ("
		  << name_slopes << ")"  << endl;

	if (hasAddresses())
	{
		string name_addresses = gnuplotGenerator::gnuplot_addresses (this, os,
		  prefix, TimeUnit, getMinimumAddress(), getMaximumAddress(), variables);
		if (name_addresses.length() > 0)
			cout << "Summary plot for region " << regionName << " ("
			  << name_addresses << ")" << endl;
	}

	for (unsigned m = 0; m < models.size(); m++)
	{
		string tmp = gnuplotGenerator::gnuplot_model (this, os, prefix,
		  models[m], TimeUnit);
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

	map<string, vector<Sample*> >::iterator i;
	for (i = used.begin(); i != used.end(); i++)
	{
		vector<Sample*> vs = (*i).second;
		for (unsigned s = 0; s < vs.size(); s++)
			allsamples.insert (vs[s]);
	}
	for (i = unused.begin(); i != unused.end(); i++)
	{
		vector<Sample*> vs = (*i).second;
		for (unsigned s = 0; s < vs.size(); s++)
			allsamples.insert (vs[s]);
	}
}

bool InstanceGroup::hasAddresses (void) const
{
	set<Sample*>::iterator s;
	for (s = allsamples.begin(); s != allsamples.end(); s++)
		if ((*s)->hasAddressReference())
			return true;
	return false;
}

unsigned long long InstanceGroup::getMinimumAddress (void) const
{
	bool anyaddress = false;
	unsigned long long minaddress;
	set<Sample*>::iterator s;
	for (s = allsamples.begin(); s != allsamples.end(); s++)
	{
		if (!anyaddress)
		{
			if ((*s)->hasAddressReference())
			{
				minaddress = (*s)->getAddressReference();
				anyaddress = true;
			}
		}
		else
		{
			if ((*s)->hasAddressReference())
				minaddress = MIN(minaddress, (*s)->getAddressReference());
		}
	}
	return minaddress;
}

unsigned long long InstanceGroup::getMaximumAddress (void) const
{
	bool anyaddress = false;
	unsigned long long maxaddress;
	set<Sample*>::iterator s;
	for (s = allsamples.begin(); s != allsamples.end(); s++)
	{
		if (!anyaddress)
		{
			if ((*s)->hasAddressReference())
			{
				maxaddress = (*s)->getAddressReference();
				anyaddress = true;
			}
		}
		else
		{
			if ((*s)->hasAddressReference())
				maxaddress = MAX(maxaddress, (*s)->getAddressReference());
		}
	}
	return maxaddress;
}
