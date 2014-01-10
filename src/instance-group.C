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

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "instance-group.H"
#include "interpolation-results.H"

using namespace std;

InstanceGroup::InstanceGroup (string name, unsigned id, string groupname) :
	regionName (name), numGroup (id), groupName (groupname)
{
}

void InstanceGroup::add (Instance *i)
{
	Instances.push_back (i);
}

unsigned InstanceGroup::numInstances (void) const
{
	return Instances.size();
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

void InstanceGroup::gnuplot_single (const ObjectSelection *os, const string &prefix,
	const string &counter, InterpolationResults *idata, const string & TimeUnit)
{
	string fintname = prefix + "." + os->toString(false, "any") 
	  + ".interpolate.csv";
	string fslname = prefix + "." + os->toString(false, "any") 
	  + ".slope.csv";
	string fdname = prefix + "." + os->toString(false, "any") 
	  + ".dump.csv";
	string gname = prefix + "." + os->toString (false, "any") + "." +
	  common::removeUnwantedChars(regionName) + "." + 
	  common::removeSpaces (groupName) + "." +
	  counter + ".gnuplot";
	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return;
	}

	double m = mean(TimeUnit);

	if (used.count(counter) == 0 || unused.count(counter) == 0)
	{
		cerr << "Invalid used/unused samples hash count" << endl;
		return;
	}

	vector<Sample*> usedSamples = used[counter];
	vector<Sample*> unusedSamples = unused[counter];

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "# set term postscript eps solid color;" << endl <<
	  "# set term pdfcairo solid color lw 2 font \",9\";" << endl <<
	  "# set term png size 800,600;" << endl <<
	  "set datafile separator \";\";" << endl <<
	  "set key bottom outside center horizontal samplen 1 font \",9\";" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:1];" << endl <<
	  "set y2range [0:*];" << endl <<
	  "set ytics nomirror;" << endl <<
	  "set y2tics nomirror;" << endl <<
	  "set x2tics nomirror format \"%.2f\";" << endl <<
	  "set ylabel 'Normalized " << counter << "';" << endl;

	if (TimeUnit == common::DefaultTimeUnit)
		gplot << "set xlabel 'Time (in ms)';" << endl;
	else
		gplot << "set xlabel '" << TimeUnit << " (in Mevents)';" << endl;

  	gplot << "set xtics nomirror format \"%.2f\";" << endl
	  << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i*(m/1000000)/5;
	gplot << ");" << endl
	  << "set xrange [0:" << (m / 1000000) << "];" << endl;

	if (common::isMIPS(counter))
	  gplot << "set y2label 'MIPS';" << endl;
	else	
	  gplot << "set y2label '" << counter << " rate (in Mevents/s)';" << endl;

	if (TimeUnit == common::DefaultTimeUnit)
		gplot << "set title \"" << os->toString (true) << " - " << groupName
		  << " - " << regionName << "\\nDuration = " << (m/1000000) << " ms, Counter = " 
		  << (idata->getAvgCounterValue() / 1000000) << " Mevents\";" << endl;
	else
		gplot << "set title \"" << os->toString (true) << " - " << groupName
		  << " - " << regionName << "\\nDuration = " << (m/1000000) << " Mevents, Counter = " 
		  << (idata->getAvgCounterValue() / 1000000) << " Mevents\";" << endl;

	/* Mark the mean rate in the plot */
	gplot << "# Mean rate" << endl;
	if (TimeUnit == common::DefaultTimeUnit)
		gplot << endl 
		  << "set label \"\" at first " << (m/1000000)
		  << ", second " << (idata->getAvgCounterValue() / 1000)/(m/1000000)
		  << " point pt 3 ps 2 lc rgbcolor \"#707070\";" << endl;
	else
		gplot << endl 
		  << "set label \"\" at first " << (m/1000000)
		  << ", second " << (idata->getAvgCounterValue()/m)
		  << " point pt 3 ps 2 lc rgbcolor \"#707070\";" << endl;

	/* If the instance-group has more than the regular 0..1 breakpoints,
	   add this into the plot */
	gplot << "# Breakpoints" << endl;
	vector<double> brks = idata->getBreakpoints();
	if (brks.size () > 2)
	{
		gplot << endl;
		gplot << fixed << setprecision(8);
		for (unsigned u = 1; u < brks.size()-1; u++) /* Skip 0 and 1 */
			gplot << "set arrow from graph "
			  << brks[u] << ",0 to graph " << brks[u]
			  << ",1 nohead ls 0 lc rgb 'black' lw 2 front;" << endl;

		for (unsigned u = 1; u < brks.size(); u++)
		{
			double half = brks[u-1] + (brks[u]-brks[u-1])/2;
			gplot << "set label \"Phase " << u << "\" at graph " << half 
			  << ",0.95 rotate by -90 front textcolor rgb '#808080';" << endl;
		}
		gplot << fixed << setprecision(2);
	}
	else
		gplot << endl << "# Unneeded phases separators, nb. breakpoints = " << brks.size() << endl;

	/* Generate functions to filter the .csv */
	gplot << "# Data accessors to CSV" << endl;
	gplot << endl << "sampleexcluded(ret,c,r,g,t) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << "  && t eq 'e') ? ret : NaN;" << endl
	  << "sampleunused(ret,c,r,g,t) = (c eq '" << counter << "' && r eq '" << regionName << "' && g ==  " << numGroup << " && t eq 'un') ? ret : NaN;" << endl
	  << "sampleused(ret,c,r,g,t) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << " && t eq 'u') ? ret : NaN;" << endl
	  << "interpolation(ret,c,r,g) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << " ) ? ret : NaN;" << endl
	  << "slope(ret,c,r,g) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << " ) ? ret : NaN;" << endl << endl;

	bool coma = false;
	/* Generate the plot command */
	gplot << "# Plot command" << endl;
	gplot << "plot \\" << endl << 
	  "NaN ti 'Mean " << counter << " rate' w points pt 3 lc rgbcolor \"#707070\" ,\\" << endl;
	if (numExcludedSamples (counter) > 0)
	{
		gplot << "'" << fdname << "' u 4:(sampleexcluded($6, strcol(5), strcol(2), $3, strcol(1))) ti 'Excluded samples (" << numExcludedSamples(counter) << ")' axes x2y1 w points pt 3 lc rgbcolor '#A0A0A0'";
		coma = true;
	}
	if (Instances.size() > 0)
	{
		if (coma)
			gplot << ",\\" << endl;
		if (unusedSamples.size() > 0)
		{
			gplot << "'" << fdname << "' u 4:(sampleunused($6, strcol(5), strcol(2), $3, strcol(1))) ti 'Unused samples (" << unusedSamples.size() << ")' axes x2y1 w points pt 3 lc rgbcolor '#FFA0A0'";
			coma = true;
		}
		else
			coma = false;
		if (usedSamples.size() > 0)
		{
			if (coma)
				gplot << ",\\" << endl;
			gplot << "'" << fdname << "' u 4:(sampleused($6, strcol(5), strcol(2), $3, strcol(1))) ti 'Used samples (" << usedSamples.size() << ")' axes x2y1 w points pt 3 lc rgbcolor '#FF0000'";
			coma = true;
		}

		if (coma)
			gplot << ",\\" << endl;

		gplot << "'" << fintname << "' u 4:(interpolation($5, strcol(3), strcol(1), $2)) ti 'Fitting [" << idata->getInterpolationDetails() << "]' axes x2y1 w lines lw 3 lc rgbcolor '#00FF00'";

		if (idata->isSlopeCalculated())
		{
			gplot << ",\\" << endl << "'" << fslname << "' u 4:(slope($5, strcol(3), strcol(1), $2)) ti 'Counter rate' axes x2y2 w lines lw 3 lc rgbcolor '#0000FF'";
		}

	}
	gplot << ";" << endl;

	gplot.close();
}

string InstanceGroup::gnuplot_slopes (const ObjectSelection *os, const string &prefix,
	bool per_instruction, const string & TimeUnit)
{
	string fslname = prefix + "." + os->toString(false, "any") + ".slope.csv";
	string gname = prefix + "." + os->toString (false, "any") + "." +
	  common::removeUnwantedChars(regionName) + "." + 
	  common::removeSpaces (groupName);
	if (per_instruction)
		gname += ".ratio_per_ins.gnuplot";
	else
		gname += ".slopes.gnuplot";
	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return "";
	}

	double m = mean();

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "# set term postscript eps solid color;" << endl <<
	  "# set term pdfcairo solid color lw 2 font \",9\";" << endl <<
	  "# set term png size 800,600;" << endl <<
	  "set datafile separator \";\";" << endl <<
	  "set key bottom outside center horizontal samplen 1 font \",9\";" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:*];" << endl <<
	  "set x2tics nomirror format \"%.2f\";" << endl;

	if (TimeUnit == common::DefaultTimeUnit)
	{
		gplot << "set xlabel 'Time (in ms)';" << endl
	  	  << "set xtics nomirror format \"%.2f\";" << endl
		  << "set xtics ( 0.0 ";
		for (int i = 1; i <= 5; i++)
			gplot << ", " << i*(m/1000000)/5;
		gplot << ");" << endl
		  << "set xrange [0:" << (m / 1000000) << "];" << endl;
	}
	else
	{
		gplot << "set xlabel 'Normalized " << TimeUnit << "';" << endl
		  << "set xrange [0:1];" << endl 
	  	  << "set xtics nomirror format \"%.2f\";" << endl;
	}

	if (per_instruction)
	{
		gplot << "set ylabel 'Counter ratio per instruction';" << endl <<
		  "set y2label 'MIPS';" << endl <<
		  "set ytics nomirror;" << endl <<
		  "set y2tics nomirror;" << endl;
	}
	else
		gplot << "set ylabel 'Performance counter rate (in Mevents/s)';" << endl <<
	  "set ytics mirror;" << endl;

	gplot << "set title \"" << os->toString (true) << " - " << groupName 
	  <<  " - " << regionName << "\\nDuration = " << (m/1000000) << " ms\";" 
	  << endl;

	/* If the instance-group has more than the regular 0..1 breakpoints,
	   add this into the plot */
	if (Breakpoints.size () > 2)
	{
		gplot << endl;
		gplot << fixed << setprecision(8);
		for (unsigned u = 1; u < Breakpoints.size()-1; u++) /* Skip 0 and 1 */
			gplot << "set arrow from graph "
			  << Breakpoints[u] << ",0 to graph " << Breakpoints[u]
			  << ",1 nohead ls 0 lc rgb 'black' lw 2 front;" << endl;

		for (unsigned u = 1; u < Breakpoints.size(); u++)
		{
			double half = Breakpoints[u-1] + (Breakpoints[u]-Breakpoints[u-1])/2;
			gplot << "set label \"Phase " << u << "\" at graph " << half 
			  << ",0.95 rotate by -90 front textcolor rgb '#808080';" << endl;
		}
		gplot << fixed << setprecision(2);
	}
	else
		gplot << endl << "# Unneeded phases separators, nb. breakpoints = " << Breakpoints.size() << endl;

	/* Generate functions to filter the .csv */
	map<string, InterpolationResults*>::iterator it;
	for (it = interpolated.begin(); it != interpolated.end(); it++)
		if ((*it).second->isSlopeCalculated())
		{
			string counter_gnuplot = (*it).first;
			for (unsigned u = 0; u < counter_gnuplot.length(); u++)
				if (counter_gnuplot[u] == ':')
					counter_gnuplot[u] = '_';

			gplot << "slope_" << counter_gnuplot << "(ret,c,r,g) = (c eq '" << (*it).first
			  << "' && r eq '" << regionName << "' && g == " << numGroup << " ) ? ret : NaN;" << endl;
			if (!common::isMIPS((*it).first))
				gplot << "ratio_" << counter_gnuplot << "(ret,c,r,g) = (c eq '" << (*it).first
				  << "_per_ins' && r eq '" << regionName << "' && g == " << numGroup << " ) ? ret : NaN;" << endl;
		}
	gplot << endl;

	/* Generate the plot command */
	unsigned count = 0;
	for (it = interpolated.begin(); it != interpolated.end(); it++)
	{
		if ((*it).second->isSlopeCalculated())
		{
			if (count == 0)
				gplot << "plot \\" << endl;
			else
				gplot << ",\\" << endl;

			string counter_gnuplot = (*it).first;
			for (unsigned u = 0; u < counter_gnuplot.length(); u++)
				if (counter_gnuplot[u] == ':')
					counter_gnuplot[u] = '_';

			if (common::isMIPS((*it).first))
	  		{
				if (per_instruction)
					gplot << "'" << fslname << "' u 4:(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y2 w lines lw 3";
				else
					gplot << "'" << fslname << "' u 4:(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y1 w lines lw 3";
			}
			else
			{
				if (per_instruction)
					gplot << "'" << fslname << "' u 4:(ratio_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti '" << (*it).first
					  << "/ins' axes x2y1 w lines lw 3";
				else
					gplot << "'" << fslname << "' u 4:(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti '" << (*it).first
					  << "' axes x2y1 w lines lw 3";
			}
			count++;
		}
	}
	gplot << ";" << endl;

	gplot.close();

	return gname;
}

string InstanceGroup::gnuplot_model (const ObjectSelection *os,
	const string & prefix, const Model *m, const string & TimeUnit)
{
	string fslname = prefix + "." + os->toString(false, "any") + ".slope.csv";
	string gname = prefix + "." + os->toString (false, "any") + "." +
	  common::removeUnwantedChars(regionName) + "." + 
	  common::removeSpaces (groupName) + "." +
	  m->getName() + ".gnuplot";
	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return "";
	}

	double me = mean();

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "# set term postscript eps solid color;" << endl <<
	  "# set term pdfcairo solid color lw 2 font \",9\";" << endl <<
	  "# set term png size 800,600;" << endl;

	if (m->isY1Stacked())
	{
		gplot << endl <<
		  "set style histogram rowstacked;" << endl <<
		  "set style data histogram;" << endl <<
		  "set style fill solid 1 noborder;" << endl;
	}

	gplot << endl <<
	  "set datafile separator \";\";" << endl <<
	  "set key bottom outside center horizontal samplen 1 font \",9\";" << endl <<
	  "set x2range [0:1];" << endl;
	if (m->isY1Stacked())
		gplot << "set yrange [0:1];" << endl;
	else
		gplot << "set yrange [0:*];" << endl;
	gplot <<
	  "set xtics nomirror format \"%.2f\";" << endl <<
	  "set x2tics nomirror format \"%.2f\";" << endl <<
	  "set xrange [0:" << m << "];" << endl <<
	  "set xlabel 'Time (in ms)';" << endl <<
	  "set ylabel '" << m->getY1AxisName() <<  "';" << endl <<
	  "set y2label '" << m->getY2AxisName() << "';" << endl <<
	  "set ytics nomirror;" << endl <<
	  "set y2tics nomirror;" << endl;

	gplot << "set title \"Evolution for " << m->getTitleName() << " model\\n"
	  << os->toString (true) << " - " << groupName 
	  <<  " - " << regionName << "\\nDuration = " << (me/1000000) << " ms\";" 
	  << endl;

	gplot << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i*(me/1000000)/5;
	gplot << ");" << endl
	  << "set xrange [0:" << (me / 1000000) << "];" << endl;

	/* If the instance-group has more than the regular 0..1 breakpoints,
	   add this into the plot */
	if (Breakpoints.size () > 2)
	{
		gplot << endl;
		gplot << fixed << setprecision(8);
		for (unsigned u = 1; u < Breakpoints.size()-1; u++) /* Skip 0 and 1 */
			gplot << "set arrow from graph "
			  << Breakpoints[u] << ",0 to graph " << Breakpoints[u]
			  << ",1 nohead ls 0 lc rgb 'black' lw 2 front;" << endl;

		for (unsigned u = 1; u < Breakpoints.size(); u++)
		{
			double half = Breakpoints[u-1] + (Breakpoints[u]-Breakpoints[u-1])/2;
			gplot << "set label \"Phase " << u << "\" at graph " << half 
			  << ",0.95 rotate by -90 front textcolor rgb '#808080';" << endl;
		}
		gplot << fixed << setprecision(2);
	}
	else
		gplot << endl << "# Unneeded phases separators, nb. breakpoints = " << Breakpoints.size() << endl;

	/* Generate functions to filter the .csv according to the components of 
	   the model*/
	vector<ComponentModel*> vcm = m->getComponents();
	for (unsigned cm = 0; cm < vcm.size(); cm++)
	{
		const ComponentNode * cn = vcm[cm]->getComponentNode();
		gplot << "slope_" << m->getName() << "_" << vcm[cm]->getName() << 
		  "(ret,c,r,g) = (c eq '" << m->getName() << "_" << vcm[cm]->getName() <<
		  "' && r eq '" << regionName << "' && g == " << numGroup << " ) ? ret : NaN;" << endl;
	}
	gplot << endl;

	/* Generate the plot command */
	for (unsigned cm = 0; cm < vcm.size(); cm++)
	{
		if (cm == 0)
			gplot << "plot \\" << endl;
		else
			gplot << ",\\" << endl;

		const ComponentNode * cn = vcm[cm]->getComponentNode();

		string Yaxis = vcm[cm]->getPlotLocation();

		if (Yaxis == "y1" && m->isY1Stacked())
			// First grep the data we want from the CSV
			gplot << "'< grep ^\"" << regionName << ";" << numGroup << ";" << m->getName() <<
			  "_" << vcm[cm]->getName() << ";\" " << fslname << "' u (slope_" << m->getName() <<
			  "_" << vcm[cm]->getName() << "($5, strcol(3), strcol(1), $2)) ti '" << 
			  vcm[cm]->getTitleName() << "' axes x1" << Yaxis;
		else
			gplot << "'" << fslname << "' u 4:(slope_" << m->getName() << "_" <<
			  vcm[cm]->getName() << "($5, strcol(3), strcol(1), $2)) ti '" << 
			  vcm[cm]->getTitleName() << "' axes x2" << Yaxis  << " w lines lw 3";

		if (vcm[cm]->hasColor())
			gplot << " lc rgbcolor '" << vcm[cm]->getColor() << "'";
	}
	gplot << ";" << endl;

	gplot.close();

	return gname;
}

string InstanceGroup::gnuplot_addresses (const ObjectSelection *os,
	const string & prefix, const string & TimeUnit,
	unsigned long long minaddress, unsigned long long maxaddress)
{
	string fslname = prefix + "." + os->toString(false, "any") + ".slope.csv";
	string fdname = prefix + "." + os->toString(false, "any") 
	  + ".dump.csv";
	string gname = prefix + "." + os->toString (false, "any") + "." +
	  common::removeUnwantedChars(regionName) + "." + 
	  common::removeSpaces (groupName);
	gname += ".addresses.gnuplot";

	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return "";
	}

	double m = mean();

	unsigned numhexdigits_maxaddress =
	  common::numHexadecimalDigits(maxaddress);

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "# set term postscript eps solid color;" << endl <<
	  "# set term pdfcairo solid color lw 2 font \",9\";" << endl <<
	  "# set term png size 800,600;" << endl <<
	  "set datafile separator \";\";" << endl <<
	  "set key bottom outside center horizontal samplen 1 font \",9\";" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:*];" << endl <<
	  "set xtics nomirror format \"%.2f\";" << endl <<
	  "set x2tics nomirror format \"%.2f\";" << endl <<
	  "set xrange [0:" << m << "];" << endl <<
	  "set xlabel 'Time (in ms)';" << endl <<
	  "set ylabel 'Performance counter rate (in Mevents/s)';" << endl <<
	  "set ytics nomirror;" << endl <<
	  "set y2label 'Addresses referenced';" << endl;


	gplot << "set title \"" << os->toString (true) << " - " << groupName 
	  <<  " - " << regionName << "\\nDuration = " << (m/1000000) << " ms\";" 
	  << endl;

	gplot << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i*(m/1000000)/5;
	gplot << ");" << endl
	  << "set xrange [0:" << (m / 1000000) << "];" << endl;

	gplot << "set y2tics nomirror format '%0" << numhexdigits_maxaddress << "x' (" << minaddress;
	for (int i = 1; i <= 5; i++)
		gplot << ", " << minaddress+i*((maxaddress-minaddress)/5);
	gplot << ");" << endl
	  << "set y2range [" << minaddress << ":" << maxaddress << "];" << endl;

	/* If the instance-group has more than the regular 0..1 breakpoints,
	   add this into the plot */
	if (Breakpoints.size () > 2)
	{
		gplot << endl;
		gplot << fixed << setprecision(8);
		for (unsigned u = 1; u < Breakpoints.size()-1; u++) /* Skip 0 and 1 */
			gplot << "set arrow from graph "
			  << Breakpoints[u] << ",0 to graph " << Breakpoints[u]
			  << ",1 nohead ls 0 lc rgb 'black' lw 2 front;" << endl;

		for (unsigned u = 1; u < Breakpoints.size(); u++)
		{
			double half = Breakpoints[u-1] + (Breakpoints[u]-Breakpoints[u-1])/2;
			gplot << "set label \"Phase " << u << "\" at graph " << half 
			  << ",0.95 rotate by -90 front textcolor rgb '#808080';" << endl;
		}
		gplot << fixed << setprecision(2);
	}
	else
		gplot << endl << "# Unneeded phases separators, nb. breakpoints = " << Breakpoints.size() << endl;

	/* Generate functions to filter the .csv */
	map<string, InterpolationResults*>::iterator it;
	for (it = interpolated.begin(); it != interpolated.end(); it++)
		if ((*it).second->isSlopeCalculated())
		{
			string counter_gnuplot = (*it).first;
			for (unsigned u = 0; u < counter_gnuplot.length(); u++)
				if (counter_gnuplot[u] == ':')
					counter_gnuplot[u] = '_';

			gplot << "slope_" << counter_gnuplot << "(ret,c,r,g) = (c eq '" << (*it).first
			  << "' && r eq '" << regionName << "' && g == " << numGroup << " ) ? ret : NaN" << endl;
		}
	gplot << "address_L1(ret,w,r,g) = (w == 1 && r eq '" << regionName <<
	  "' && g == 0) ? ret : NaN;" << endl;
	gplot << "address_LFB(ret,w,r,g) = (w == 2 && r eq '" << regionName <<
	  "' && g == 0) ? ret : NaN;" << endl;
	gplot << "address_L2(ret,w,r,g) = (w == 3 && r eq '" << regionName <<
	  "' && g == 0) ? ret : NaN;" << endl;
	gplot << "address_L3(ret,w,r,g) = (w == 4 && r eq '" << regionName <<
	  "' && g == 0) ? ret : NaN;" << endl;
	gplot << "address_RCACHE(ret,w,r,g) = (w == 5 && r eq '" << regionName <<
	  "' && g == 0) ? ret : NaN;" << endl;
	gplot << "address_DRAM(ret,w,r,g) = (w == 6 && r eq '" << regionName <<
	  "' && g == 0) ? ret : NaN;" << endl;
	gplot << "address_OTHER(ret,w,r,g) = (w == 0 && r eq '" << regionName <<
	  "' && g == 0) ? ret : NaN;" << endl;

	/* Generate the plot command */
	unsigned count = 0;
	for (it = interpolated.begin(); it != interpolated.end(); it++)
	{
		if ((*it).second->isSlopeCalculated())
		{
			if (count == 0)
				gplot << "plot \\" << endl;
			else
				gplot << ",\\" << endl;

			string counter_gnuplot = (*it).first;
			for (unsigned u = 0; u < counter_gnuplot.length(); u++)
				if (counter_gnuplot[u] == ':')
					counter_gnuplot[u] = '_';

			if (common::isMIPS((*it).first))
				gplot << "'" << fslname << "' u 4:(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y1 w lines lw 3";
			else
				gplot << "'" << fslname << "' u 4:(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti '" << (*it).first
				      << "' axes x2y1 w lines lw 3";

			count++;
		}
	}
	gplot << ",\\" << endl << 
	  "'" << fdname <<"' u 4:(address_L1($5, $6, strcol(2), $3)) ti 'L1 reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u 4:(address_LFB($5, $6, strcol(2), $3)) ti 'LFB reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u 4:(address_L2($5, $6, strcol(2), $3)) ti 'L2 reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u 4:(address_L3($5, $6, strcol(2), $3)) ti 'L3 reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u 4:(address_RCACHE($5, $6, strcol(2), $3)) ti 'RCache reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u 4:(address_DRAM($5, $6, strcol(2), $3)) ti 'DRAM reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u 4:(address_OTHER($5, $6, strcol(2), $3)) ti 'Other reference' axes x2y2 w points pt 3;" << endl;

	gplot.close();

	return gname;
}

void InstanceGroup::gnuplot (const ObjectSelection *os, const string & prefix,
	const vector<Model*> & models, const string &TimeUnit)
{
	bool has_instruction_counter = false;
	map<string, InterpolationResults*>::iterator it;

	/* Dump single plots first */
	for (it = interpolated.begin(); it != interpolated.end(); it++)
	{
		has_instruction_counter |= common::isMIPS((*it).first);
		gnuplot_single (os, prefix, (*it).first, (*it).second, TimeUnit);
	}

	/* If has instruction counter, generate this in addition to .slopes */
	string name_slopes, name_inst_ctr;
	if (has_instruction_counter)
		name_inst_ctr = gnuplot_slopes (os, prefix, true, TimeUnit);
	name_slopes = gnuplot_slopes (os, prefix, false, TimeUnit);

	/* If has instruction counter, let the new plot be the summary, otherwise
	   let the .slopes be the summary */
	if (has_instruction_counter && name_inst_ctr.length() > 0)
		cout << "Summary plot for region " << regionName << " ("
		  << name_inst_ctr << ")" << endl;
	if (name_slopes.length() > 0)
		cout << "Summary plot for region " << regionName << " ("
		  << name_slopes << ")"  << endl;

	/* Check if address sampling should be emitted */
	bool anyaddress = false;
	unsigned long long minaddress, maxaddress;
	set<Sample*>::iterator s;
	for (s = allsamples.begin(); s != allsamples.end(); s++)
	{
		if (!anyaddress)
		{
			if ((*s)->hasAddressReference())
			{
				minaddress = maxaddress = (*s)->getAddressReference();
				anyaddress = true;
			}
		}
		else
		{
			if ((*s)->hasAddressReference())
			{
				minaddress = MIN(minaddress, (*s)->getAddressReference());
				maxaddress = MAX(maxaddress, (*s)->getAddressReference());
			}
		}
	}

	if (anyaddress)
	{
		string name_addresses = gnuplot_addresses (os, prefix, TimeUnit,
		  minaddress, maxaddress);
		if (name_addresses.length() > 0)
			cout << "Summary plot for region " << regionName << " ("
			  << name_addresses << ")" << endl;
	}

	for (unsigned m = 0; m < models.size(); m++)
	{
		string tmp = gnuplot_model (os, prefix, models[m], TimeUnit);
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

