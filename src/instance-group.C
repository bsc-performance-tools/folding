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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/common.C $
 | 
 | @last_commit: $Date: 2013-05-24 16:08:28 +0200 (dv, 24 mai 2013) $
 | @version:     $Revision: 1764 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char __attribute__ ((unused)) rcsid[] = "$Id: common.C 1764 2013-05-24 14:08:28Z harald $";

#include "common.H"

#include <math.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "instance-group.H"
#include "interpolation-results.H"

using namespace std;

InstanceGroup::InstanceGroup (string name, unsigned id, string groupname)
{
	regionName = name;
	numGroup = id;
	groupName = groupname;
}

void InstanceGroup::add (Instance *i)
{
	Instances.push_back (i);
}

unsigned InstanceGroup::numInstances (void)
{
	return Instances.size();
}

unsigned InstanceGroup::numSamples (void)
{
	unsigned tmp = 0;
	for (unsigned u = 0; u < Instances.size(); u++)
		tmp += Instances[u]->Samples.size();

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

unsigned InstanceGroup::numExcludedInstances (void)
{
	return excludedInstances.size();
}

unsigned InstanceGroup::numExcludedSamples (string counter)
{
	unsigned total = 0;

	for (unsigned i = 0; i < excludedInstances.size(); i++)
		for (unsigned u = 0; u < excludedInstances[i]->Samples.size(); u++)
		{
			Sample *s = excludedInstances[i]->Samples[u];
			if (s->nCounterValue.count (counter) > 0)
				total++;
		}

	return total;
}

unsigned InstanceGroup::numExcludedSamples (void)
{
	unsigned tmp = 0;
	for (unsigned u = 0; u < excludedInstances.size(); u++)
		tmp += excludedInstances[u]->Samples.size();

	return tmp;
}

unsigned long long InstanceGroup::mean (void)
{
	unsigned long long tmp = 0;

	if (Instances.size() == 0)
		return tmp;

	for (unsigned u = 0; u < Instances.size(); u++)
		tmp += Instances[u]->duration;

	return tmp / Instances.size();
}

unsigned long long InstanceGroup::median (void)
{
	vector<unsigned long long> durations;

	if (Instances.size() == 0)
		return 0;

	for (unsigned u = 0; u < Instances.size(); u++)
		durations.push_back (Instances[u]->duration);

	sort (durations.begin(), durations.end());

	return durations[Instances.size() / 2];
}

double InstanceGroup::stdev (void)
{
	double tmp = 0;
	double _mean;

	if (Instances.size() <= 1)
		return tmp;

	_mean = this->mean();
	for (unsigned u = 0; u < Instances.size(); u++)
	{
		double d = Instances[u]->duration;
		tmp += ( (d-_mean) * (d-_mean) );
	}

	return sqrt ( tmp / (Instances.size() -1) );
}

unsigned long long InstanceGroup::MAD (void)
{
	vector<unsigned long long> absmediandiff;
	unsigned long long _median;

	if (Instances.size() == 0)
		return 0;

	_median = this->median();
	for (unsigned u = 0; u < Instances.size(); u++)
		if (Instances[u]->duration > _median)
			absmediandiff.push_back (Instances[u]->duration - _median);
		else
			absmediandiff.push_back (_median - Instances[u]->duration);

	sort (absmediandiff.begin(), absmediandiff.end());

	return absmediandiff[Instances.size() / 2];
}

void InstanceGroup::removePreviousData (ObjectSelection *os, string prefix)
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

void InstanceGroup::dumpInterpolatedData (ObjectSelection *os, string prefix)
{
	string fintname = prefix + "." + os->toString(false, "any") +
	  ".interpolate.csv";
	string fslname = prefix + "." + os->toString(false, "any") +
	  ".slope.csv";

	if (Instances.size() > 0)
	{
		ofstream int_data, sl_data;
		int_data.open (fintname.c_str(), std::ofstream::app);
		sl_data.open (fslname.c_str(), std::ofstream::app);
		map<string, InterpolationResults*>::iterator it;
		for (it = interpolated.begin(); it != interpolated.end(); it++)
		{
			InterpolationResults *ir = (*it).second;
			unsigned count = ir->getCount();
			double d_count = (double) count;

			string counter = (*it).first;
			double *data_i = ir->getInterpolationResultsPtr();
			double *data_s = ir->getSlopeResultsPtr();
			for (unsigned u = 0; u < count; u++)
			{
				double d_j = (double) u;
				int_data << regionName << ";" << numGroup << ";" << counter << ";" << d_j / d_count << ";" << data_i[u] << endl;
				sl_data << regionName << ";" << numGroup << ";" << counter << ";" << d_j / d_count << ";" << data_s[u] << endl;
			}
		}
		int_data.close ();
		sl_data.close ();
	}
}

void InstanceGroup::dumpData (ObjectSelection *os, string prefix)
{
	string fname = prefix + "." + os->toString(false, "any") +
	  ".dump.csv";

	ofstream odata;
	odata.open (fname.c_str(), std::ofstream::app);

	if (Instances.size() > 0)
	{
		map<string, vector<Sample*> >::iterator it;
		for (it = used.begin(); it != used.end(); it++)
		{
			vector<Sample*> usedSamples = (*it).second;
			for (unsigned u = 0; u < usedSamples.size(); u++)
				odata << "u" << ";" << regionName << ";" << numGroup << ";" << usedSamples[u]->nTime << ";" << (*it).first << ";" << usedSamples[u]->nCounterValue[(*it).first] << endl;
		}
		for (it = unused.begin(); it != unused.end(); it++)
		{
			vector<Sample*> unusedSamples = (*it).second;
			for (unsigned u = 0; u < unusedSamples.size(); u++)
				odata << "un" << ";" << regionName << ";" << numGroup << ";" << unusedSamples[u]->nTime << ";" << (*it).first << ";" << unusedSamples[u]->nCounterValue[(*it).first] << endl;
		}
	}

	if (excludedInstances.size() > 0)
	{
		for (unsigned i = 0; i < excludedInstances.size(); i++)
		{
			Instance *inst = excludedInstances[i];
			for (unsigned u = 0; u < inst->Samples.size(); u++)
			{
				double time = inst->Samples[u]->nTime;
				map<string, double> counters = inst->Samples[u]->nCounterValue;
				map<string, double>::iterator it = counters.begin();
				for ( ; it != counters.end(); it++)
					odata << "e" << ";" << regionName << ";" << numGroup << ";" << time << ";" << (*it).first << ";" << (*it).second << endl;
			}
		}
	}

	odata.close ();
}

void InstanceGroup::gnuplot_single (ObjectSelection *os, string prefix,
	string counter, InterpolationResults *idata)
{
	string fintname = prefix + "." + os->toString(false, "any") 
	  + ".interpolate.csv";
	string fslname = prefix + "." + os->toString(false, "any") 
	  + ".slope.csv";
	string fdname = prefix + "." + os->toString(false, "any") 
	  + ".dump.csv";
	string gname = prefix + "." + common::removeUnwantedChars(regionName) + "." + 
	  os->toString (false, "any") + "." + common::removeSpaces (groupName) +
	  "." + counter + ".gnuplot";
	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return;
	}

	double m = mean();

	if (used.count(counter) == 0 || unused.count(counter) == 0)
	{
		cerr << "Invalid used/unused samples hash count" << endl;
		return;
	}

	vector<Sample*> usedSamples = used[counter];
	vector<Sample*> unusedSamples = unused[counter];

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "# set term postscript eps solid color" << endl <<
	  "# set term pdfcairo solid color lw 2 font \",9\"" << endl <<
	  "# set term png size 800,600" << endl <<
	  "set datafile separator \";\"" << endl <<
	  "set key bottom outside center horizontal samplen 1 font \",9\"" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:1];" << endl <<
	  "set y2range [0:*];" << endl <<
	  "set ytics nomirror; " << endl <<
	  "set y2tics nomirror; " << endl <<
	  "set xtics nomirror; " << endl <<
	  "set x2tics nomirror; " << endl <<
	  "set xrange [0:" << m << "];" << endl <<
	  "set xlabel 'Time (in ms)'" << endl <<
	  "set ylabel 'Normalized " << counter << "';" << endl;
	if (common::isMIPS(counter))
	  gplot << "set y2label 'MIPS';" << endl;
	else	
	  gplot << "set y2label '" << counter << " rate (in Mevents/s)';" << endl;

	gplot << "set title \"" << os->toString (true) << " - " << groupName
	  << " - " << regionName << "\\nDuration = " << (m/1000000) << " ms, Counter = " 
	  << (idata->getAvgCounterValue() / 1000) << " Kevents\"" << endl;

	gplot << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i*(m/1000000)/5;
	gplot << ");" << endl
	  << "set xrange [0:" << (m / 1000000) << "]" << endl;

	/* If the instance-group has more than the regular 0..1 breakpoints,
	   add this into the plot */
	vector<double> brks = idata->getBreakpoints();
	if (brks.size () > 2)
	{
		gplot << endl;
		gplot << fixed << setprecision(8);
		for (unsigned u = 1; u < brks.size()-1; u++) /* Skip 0 and 1 */
			gplot << "set arrow from graph "
			  << brks[u] << ",0 to graph " << brks[u]
			  << ",1 nohead ls 0 lc rgb 'black' lw 2 front" << endl;

		for (unsigned u = 1; u < brks.size(); u++)
		{
			double half = brks[u-1] + (brks[u]-brks[u-1])/2;
			gplot << "set label \"Phase " << u << "\" at graph " << half 
			  << ",0.95 rotate by -90 front textcolor rgb '#808080'" << endl;
		}
		gplot << fixed << setprecision(2);
	}
	else
		gplot << endl << "# Unneeded phases separators, nb. breakpoints = " << brks.size() << endl;

	/* Generate functions to filter the .csv */
	gplot << endl << "sampleexcluded(ret,c,r,g,t) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << "  && t eq 'e') ? ret : NaN;" << endl
	  << "sampleunused(ret,c,r,g,t) = (c eq '" << counter << "' && r eq '" << regionName << "' && g ==  " << numGroup << " && t eq 'un') ? ret : NaN;" << endl
	  << "sampleused(ret,c,r,g,t) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << " && t eq 'u') ? ret : NaN;" << endl
	  << "interpolation(ret,c,r,g) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << " ) ? ret : NaN;" << endl
	  << "slope(ret,c,r,g) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << " ) ? ret : NaN;" << endl << endl;

	bool coma = false;
	/* Generate the plot command */
	gplot << "plot \\" << endl;
	if (numExcludedSamples (counter) > 0)
	{
		gplot << "'" << fdname << "' u 4:(sampleexcluded($6, strcol(5), strcol(2), $3, strcol(1))) ti 'Excluded samples (" << numExcludedSamples(counter) << ")' axes x2y1 w points lc rgbcolor '#A0A0A0'";
		coma = true;
	}
	if (Instances.size() > 0)
	{
		if (coma)
			gplot << ",\\" << endl;
		if (unusedSamples.size() > 0)
		{
			gplot << "'" << fdname << "' u 4:(sampleunused($6, strcol(5), strcol(2), $3, strcol(1))) ti 'Unused samples (" << unusedSamples.size() << ")' axes x2y1 w points lc rgbcolor '#FFA0A0'";
			coma = true;
		}
		else
			coma = false;
		if (usedSamples.size() > 0)
		{
			if (coma)
				gplot << ",\\" << endl;
			gplot << "'" << fdname << "' u 4:(sampleused($6, strcol(5), strcol(2), $3, strcol(1))) ti 'Used samples (" << usedSamples.size() << ")' axes x2y1 w points lc rgbcolor '#FF0000'";
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

void InstanceGroup::gnuplot_slopes (ObjectSelection *os, string prefix)
{
	string fslname = prefix + "." + os->toString(false, "any") + ".slope.csv";
	string gname = prefix + "." + common::removeUnwantedChars(regionName) + "." + 
	  os->toString (false, "any") + "." + common::removeSpaces (groupName) +
	  ".slopes.gnuplot";
	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return;
	}

	double m = mean();

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "# set term postscript eps solid color" << endl <<
	  "# set term pdfcairo solid color lw 2 font \",9\"" << endl <<
	  "# set term png size 800,600" << endl <<
	  "set datafile separator \";\"" << endl <<
	  "set key bottom outside center horizontal samplen 1 font \",9\"" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:*];" << endl <<
	  "set ytics mirror; " << endl <<
	  "set xtics nomirror; " << endl <<
	  "set x2tics nomirror; " << endl <<
	  "set xrange [0:" << m << "];" << endl <<
	  "set xlabel 'Time (in ms)'" << endl <<
	  "set ylabel 'Performance counter rate (in Mevents/s)';" << endl;

	gplot << "set title \"" << os->toString (true) << " - " << groupName 
	  <<  " - " << regionName << "\\nDuration = " << (m/1000000) << " ms\"" 
	  << endl;

	gplot << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i*(m/1000000)/5;
	gplot << ");" << endl
	  << "set xrange [0:" << (m / 1000000) << "]" << endl;

	/* If the instance-group has more than the regular 0..1 breakpoints,
	   add this into the plot */
	if (Breakpoints.size () > 2)
	{
		gplot << endl;
		gplot << fixed << setprecision(8);
		for (unsigned u = 1; u < Breakpoints.size()-1; u++) /* Skip 0 and 1 */
			gplot << "set arrow from graph "
			  << Breakpoints[u] << ",0 to graph " << Breakpoints[u]
			  << ",1 nohead ls 0 lc rgb 'black' lw 2 front" << endl;

		for (unsigned u = 1; u < Breakpoints.size(); u++)
		{
			double half = Breakpoints[u-1] + (Breakpoints[u]-Breakpoints[u-1])/2;
			gplot << "set label \"Phase " << u << "\" at graph " << half 
			  << ",0.95 rotate by -90 front textcolor rgb '#808080'" << endl;
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
				gplot << "'" << fslname << "' u 4:(slope_" << counter_gnuplot
				  << "($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y2 w lines lw 3";
			else
				gplot << "'" << fslname << "' u 4:(slope_" << counter_gnuplot
				  << "($5, strcol(3), strcol(1), $2)) ti '" << (*it).first
				  << "' axes x2y2 w lines lw 3";
			count++;
		}
	}
	gplot << ";" << endl;

	gplot.close();
}

void InstanceGroup::gnuplot (ObjectSelection *os, string prefix)
{
	map<string, InterpolationResults*>::iterator it;
	for (it = interpolated.begin(); it != interpolated.end(); it++)
		gnuplot_single (os, prefix, (*it).first, (*it).second);
	gnuplot_slopes (os, prefix);
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
