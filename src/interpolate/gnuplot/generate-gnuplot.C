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
#include "generate-gnuplot.H"
#include "generate-gnuplot-callstack.H"
#include "generate-gnuplot-references.H"

#include <assert.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stack>

#include <string.h>

#include "prv-types.H"
#include "pcf-common.H"

string gnuplotGenerator::ShortenCounterString (const string &ctr)
{
	if (ctr.substr (0, strlen("RESOURCE_STALLS")) == string("RESOURCE_STALLS"))
	{
		// Skip RESOURCE_ PART
		return ctr.substr (strlen("RESOURCE_"));
	}
	else if (ctr.substr (0, strlen("PAPI_")) == string ("PAPI_"))
	{
		// Skip PAPI_
		return ctr.substr (strlen("PAPI_"));
	}
	else
		return ctr;
}

void gnuplotGenerator::gnuplot_single (
	InstanceGroup *ig,
	const ObjectSelection *os,
	const string &prefix,
	const string &counter,
	InterpolationResults *idata,
	const string & TimeUnit,
#if defined(MEMORY_ANALYSIS)
	const vector<DataObject*> & variables,
#else
	const vector<DataObject*> & ,
#endif
	UIParaverTraceConfig *pcf
)
{
	string regionName = ig->getRegionName();
	string groupName = ig->getGroupName();
	unsigned numGroup = ig->getNumGroup();
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

	double m = ig->mean (TimeUnit);

	map<string, vector<Sample*> > used, unused;
	used = ig->getSamples();
	unused = ig->getUnusedSamples();

	if (used.count(counter) == 0 || unused.count(counter) == 0)
	{
		cerr << "Invalid used/unused samples hash count" << endl;
		return;
	}

	vector<Sample*> usedSamples = used[counter];
	vector<Sample*> unusedSamples = unused[counter];

	/* Generate the pre-info */
	gplot << fixed << setprecision(2) <<
	  "X_LIMIT=" << m / 1000000 << " # Do not touch this" << endl <<
	  "FACTOR=1" << " # Do not touch this" << endl << endl <<
	  "set datafile separator \";\";" << endl << endl <<
	  "# set term postscript eps solid color;" << endl <<
	  "# set term pdfcairo solid color lw 2;" << endl <<
	  "# set term png size 800,600;" << endl << 
	  "# set term wxt size 800,600;" << endl << endl;

	string TITLE;
	stringstream ssDuration, ssCounterAvg;
	ssDuration << fixed << setprecision(2) << m/1000000;
	ssCounterAvg << fixed << setprecision(2) << idata->getAvgCounterValue() / 1000000;

	if (TimeUnit == common::DefaultTimeUnit)
		TITLE = "\"" + os->toString (true) + " - " + groupName + " - "
		  + regionName + "\\nDuration = " + ssDuration.str() + " ms, Counter = " 
		  + ssCounterAvg.str() + " Mevents\";";
	else
		TITLE = "\"" + os->toString (true) + " - " + groupName + " - " 
		  + regionName + "\\nDuration = " + ssDuration.str() + " Mevents, Counter = " 
		  + ssCounterAvg.str() + " Mevents\";";

	gplot << "set multiplot title " << TITLE << endl << endl;

	double occupied_in_multiplot = 0.;

#if defined(CALLSTACK_ANALYSIS)
#define CALLSTACK_ANALYSIS_PART 0.15
	if (ig->getHasEmittedCallstackSamples())
	{
		gnuplotGeneratorCallstack::generate (CALLSTACK_ANALYSIS_PART,
		  0.95-CALLSTACK_ANALYSIS_PART, gplot, fdname, ig, pcf);
		occupied_in_multiplot += CALLSTACK_ANALYSIS_PART;
	}
#undef CALLSTACK_ANALYSIS_PART
#endif

#if defined(MEMORY_ANALYSIS)
#define CALLSTACK_REFERENCE_PART 0.25
	if (ig->getHasEmittedAddressSamples())
	{
		gnuplotGeneratorReferences::generate (CALLSTACK_REFERENCE_PART,
		  0.95-CALLSTACK_REFERENCE_PART-occupied_in_multiplot, gplot, fdname, ig,
		  variables);
		occupied_in_multiplot += CALLSTACK_REFERENCE_PART;
	}
#undef CALLSTACK_REFERENCE_PART
#endif

	gplot << "set size 1," << 0.925-occupied_in_multiplot << ";" << endl
	      << "set origin 0,0;" << endl 
	      << "set tmargin 0; set lmargin 14; set rmargin 17;" << endl << endl;

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "set key bottom outside center horizontal samplen 1;" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:1];" << endl <<
	  "set y2range [0:*];" << endl <<
	  "set ytics nomirror format \"%.01f\";" << endl <<
	  "set y2tics nomirror format \"%g\";" << endl <<
	  "#set x2tics nomirror format \"%.02f\";" << endl <<
	  "set ylabel 'Normalized " << counter << "';" << endl;

	if (TimeUnit == common::DefaultTimeUnit)
		gplot << "set xlabel 'Time (ms)';" << endl;
	else
		gplot << "set xlabel '" << TimeUnit << " (in Mevents)';" << endl;

  	gplot << "set xtics nomirror format \"%.02f\";" << endl
	  << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i << "./5.*X_LIMIT";
	gplot << ", X_LIMIT/FACTOR);" << endl
	  << "set xrange [0:X_LIMIT*1./FACTOR];" << endl;

	if (common::isMIPS(counter))
	  gplot << "set y2label 'MIPS';" << endl;
	else	
	  gplot << "set y2label '" << counter << " rate (in Mevents/s)';" << endl;


	/* Mark the mean rate in the plot */
	gplot << endl << "# Mean rate" << endl;
	if (TimeUnit == common::DefaultTimeUnit)
		gplot
		  << "set label \"\" at first X_LIMIT*1./FACTOR"
		  << ", second " << (idata->getAvgCounterValue() / 1000)/(m/1000000)
		  << " point pt 3 ps 2 lc rgbcolor \"#707070\";" << endl;
	else
		gplot 
		  << "set label \"\" at first X_LIMIT*1./FACTOR"
		  << ", second " << (idata->getAvgCounterValue()/m)
		  << " point pt 3 ps 2 lc rgbcolor \"#707070\";" << endl;

	/* If the instance-group has more than the regular 0..1 breakpoints,
	   add this into the plot */
	gplot << endl << "# Breakpoints" << endl;
	vector<double> brks = idata->getBreakpoints();
	if (brks.size () > 2)
	{
		gplot << endl;
		gplot << fixed << setprecision(8);
		for (unsigned u = 1; u < brks.size()-1; u++) /* Skip 0 and 1 */
			gplot << "set arrow from graph FACTOR*"
			  << brks[u] << ",0 to graph " << brks[u]
			  << ",1 nohead ls 0 lc rgb 'black' lw 2 front;" << endl;

		for (unsigned u = 1; u < brks.size(); u++)
		{
			double half = brks[u-1] + (brks[u]-brks[u-1])/2;
			gplot << "set label \"Phase " << u << "\" at graph FACTOR*" << half 
			  << ",0.95 rotate by -90 front textcolor rgb '#808080';" << endl;
		}
		gplot << fixed << setprecision(2);
	}
	else
		gplot << "# Unneeded phases separators, nb. breakpoints = " << brks.size() << endl << endl;

	/* Generate functions to filter the .csv */
	gplot << "# Data accessors to CSV" << endl;
	gplot << "sampleexcluded(ret,c,r,g,t) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << "  && t eq 'e') ? ret : NaN;" << endl
	  << "sampleunused(ret,c,r,g,t) = (c eq '" << counter << "' && r eq '" << regionName << "' && g ==  " << numGroup << " && t eq 'un') ? ret : NaN;" << endl
	  << "sampleused(ret,c,r,g,t) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << " && t eq 'u') ? ret : NaN;" << endl
	  << "interpolation(ret,c,r,g) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << " ) ? ret : NaN;" << endl
	  << "slope(ret,c,r,g) = (c eq '" << counter << "' && r eq '" << regionName << "' && g == " << numGroup << " ) ? ret : NaN;" << endl << endl;

	bool coma = false;
	/* Generate the plot command */
	gplot << "# Plot command" << endl;
	gplot << "plot \\" << endl << 
	  "NaN ti 'Mean " << counter << " rate' w points pt 3 lc rgbcolor \"#707070\" ,\\" << endl;
	if (ig->numExcludedSamples (counter) > 0)
	{
		gplot << "'" << fdname << "' u ($4*FACTOR):(sampleexcluded($6, strcol(5), strcol(2), $3, strcol(1))) ti 'Excluded samples (" << ig->numExcludedSamples(counter) << ")' axes x2y1 w points pt 3 lc rgbcolor '#A0A0A0'";
		coma = true;
	}
	if (ig->numInstances())
	{
		if (coma)
			gplot << ",\\" << endl;
		if (unusedSamples.size() > 0)
		{
			gplot << "'" << fdname << "' u ($4*FACTOR):(sampleunused($6, strcol(5), strcol(2), $3, strcol(1))) ti 'Unused samples (" << unusedSamples.size() << ")' axes x2y1 w points pt 3 lc rgbcolor '#FFA0A0'";
			coma = true;
		}
		else
			coma = false;
		if (usedSamples.size() > 0)
		{
			if (coma)
				gplot << ",\\" << endl;
			gplot << "'" << fdname << "' u ($4*FACTOR):(sampleused($6, strcol(5), strcol(2), $3, strcol(1))) ti 'Used samples (" << usedSamples.size() << ")' axes x2y1 w points pt 3 lc rgbcolor '#FF0000'";
			coma = true;
		}

		if (coma)
			gplot << ",\\" << endl;

		gplot << "'" << fintname << "' u ($4*FACTOR):(interpolation($5, strcol(3), strcol(1), $2)) ti 'Fitting [" << idata->getInterpolationDetails() << "]' axes x2y1 w lines lw 3 lc rgbcolor '#00FF00'";

		if (idata->isSlopeCalculated())
			gplot << ",\\" << endl << "'" << fslname << "' u ($4*FACTOR):(slope($5, strcol(3), strcol(1), $2)) ti 'Counter rate' axes x2y2 w lines lw 3 lc rgbcolor '#0000FF'";

	}
	gplot << ";" << endl << endl
	  << "unset label;" << endl
	  << "unset arrow;" << endl << endl
	  << "unset multiplot;" << endl;

	gplot.close();
}

string gnuplotGenerator::gnuplot_slopes (
	InstanceGroup *ig,
	const ObjectSelection *os,
	const string &prefix,
	bool per_instruction,
	const string & TimeUnit,
#if defined(MEMORY_ANALYSIS)
	const vector<DataObject*> & variables,
#else
	const vector<DataObject*> & ,
#endif
	UIParaverTraceConfig *pcf
)
{
	string regionName = ig->getRegionName();
	string groupName = ig->getGroupName();
	unsigned numGroup = ig->getNumGroup();
	string fslname = prefix + "." + os->toString(false, "any") + ".slope.csv";
	string fdname = prefix + "." + os->toString(false, "any") 
	  + ".dump.csv";
	string gname = prefix + "." + os->toString (false, "any") + "." +
	  common::removeUnwantedChars(regionName) + "." + 
	  common::removeSpaces (groupName);
	if (per_instruction)
		gname += ".ratio_per_instruction.gnuplot";
	else
		gname += ".slopes.gnuplot";
	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return "";
	}

	double m = ig->mean();

	/* Generate the pre-info */
	gplot << fixed << setprecision(2) <<
	  "X_LIMIT=" << m / 1000000 << " # Do not touch this" << endl <<
	  "FACTOR=1" << " # Do not touch this" << endl << endl <<
	  "set datafile separator \";\";" << endl << endl <<
	  "# set term postscript eps solid color;" << endl <<
	  "# set term pdfcairo solid color lw 2;" << endl <<
	  "# set term png size 800,600;" << endl <<
	  "# set term wxt size 800,600;" << endl << endl;

	string TITLE = string("\"") + os->toString (true) + " - " + groupName + " - " +
	  regionName + "\"";

	gplot << "set multiplot title " << TITLE << endl << endl;

	double occupied_in_multiplot = 0.;

#if defined(CALLSTACK_ANALYSIS)
#define CALLSTACK_ANALYSIS_PART 0.20
	gnuplotGeneratorCallstack::generate (CALLSTACK_ANALYSIS_PART,
	  0.95-CALLSTACK_ANALYSIS_PART, gplot, fdname, ig, pcf);
	occupied_in_multiplot += CALLSTACK_ANALYSIS_PART;
#undef CALLSTACK_ANALYSIS_PART
#endif

#if defined(MEMORY_ANALYSIS)
#define CALLSTACK_REFERENCE_PART 0.25
	gnuplotGeneratorReferences::generate (CALLSTACK_REFERENCE_PART,
	  0.95-CALLSTACK_REFERENCE_PART-occupied_in_multiplot, gplot, fdname, ig,
	  variables);
	occupied_in_multiplot += CALLSTACK_REFERENCE_PART;
#undef CALLSTACK_REFERENCE_PART
#endif

	gplot << "set size 1," << 0.925-occupied_in_multiplot << ";" << endl
	      << "set origin 0,0;" << endl 
	      << "set tmargin 0; set lmargin 14; set rmargin 17;" << endl << endl;

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "set key bottom outside center horizontal samplen 1;" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:*];" << endl <<
	  "set y2range [0:*];" << endl <<
	  "#set x2tics nomirror format \"%.02f\";" << endl;

	if (TimeUnit == common::DefaultTimeUnit)
	{
		gplot << "set xlabel 'Time (ms)';" << endl
	  	  << "set xtics nomirror format \"%.02f\";" << endl
		  << "set xtics ( 0.0 ";
		for (int i = 1; i <= 5; i++)
			gplot << ", " << i << "./5.*X_LIMIT";
		gplot << ", X_LIMIT/FACTOR);" << endl
		  << "set xrange [0:X_LIMIT*1./FACTOR];" << endl;
	}
	else
	{
		gplot << "set xlabel 'Normalized " << TimeUnit << "';" << endl
		  << "set xrange [0:1*1./FACTOR];" << endl 
	  	  << "set xtics nomirror format \"%.02f\";" << endl;
	}

	if (per_instruction)
	{
		gplot << "set ylabel 'Counter ratio per instruction';" << endl <<
		  "set y2label 'MIPS';" << endl <<
		  "set ytics nomirror format \"%g\";" << endl <<
		  "set y2tics nomirror format \"%g\";" << endl;
	}
	else
		gplot << "set ylabel 'Performance counter rate (in Mevents/s)';" << endl <<
	  "set ytics mirror;" << endl;

	/* If the instance-group has more than the regular 0..1 breakpoints,
	   add this into the plot */
	gplot << endl << "# Breakpoints" << endl;
	vector<double> brks = ig->getInterpolationBreakpoints();
	if (brks.size () > 2)
	{
		gplot << endl;
		gplot << fixed << setprecision(8);
		for (unsigned u = 1; u < brks.size()-1; u++) /* Skip 0 and 1 */
			gplot << "set arrow from graph FACTOR*"
			  << brks[u] << ",0 to graph " << brks[u]
			  << ",1 nohead ls 0 lc rgb 'black' lw 2 front;" << endl;

		for (unsigned u = 1; u < brks.size(); u++)
		{
			double half = brks[u-1] + (brks[u]-brks[u-1])/2;
			gplot << "set label \"Phase " << u << "\" at graph FACTOR*" << half 
			  << ",0.95 rotate by -90 front textcolor rgb '#808080';" << endl;
		}
		gplot << fixed << setprecision(2);
	}
	else
		gplot << "# Unneeded phases separators, nb. breakpoints = " << brks.size() << endl << endl;

	/* Generate functions to filter the .csv */
	map<string, InterpolationResults*>::iterator it;
	map<string, InterpolationResults*> interpolated = ig->getInterpolated();  
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
					gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y2 w lines lw 3";
				else
					gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y1 w lines lw 3";
			}
			else
			{
				if (per_instruction)
					gplot << "'" << fslname << "' u ($4*FACTOR):(ratio_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti '" << ShortenCounterString((*it).first)
					  << "/ins' axes x2y1 w lines lw 3";
				else
					gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti '" << ShortenCounterString((*it).first)
					  << "' axes x2y1 w lines lw 3";
			}
			count++;
		}
	}
	gplot << ";" << endl << endl
	  << "unset label;" << endl
	  << "unset arrow;" << endl << endl
	  << "unset multiplot;" << endl;

	gplot.close();

	return gname;
}

string gnuplotGenerator::gnuplot_model (
	InstanceGroup *ig,
	const ObjectSelection *os,
	const string & prefix,
	const Model *m,
	const string & /* TimeUnit */,
#if defined(MEMORY_ANALYSIS)
	const vector<DataObject*> & variables,
#else
	const vector<DataObject*> & ,
#endif
	UIParaverTraceConfig *pcf
)
{
	string regionName = ig->getRegionName();
	string groupName = ig->getGroupName();
	unsigned numGroup = ig->getNumGroup();
	string fdname = prefix + "." + os->toString(false, "any") 
	  + ".dump.csv";
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

	map<string, InterpolationResults*> m_ctr_ir = ig->getInterpolated ();
	map<string, InterpolationResults*>::iterator m_ctr_ir_itr = m_ctr_ir.begin();
	unsigned nsteps = (*m_ctr_ir_itr).second->getCount();

	double me = ig->mean();

	/* Generate the pre-info */
	gplot << fixed << setprecision(2) <<
	  "X_LIMIT=" << me / 1000000 << " # Do not touch this" << endl <<
	  "FACTOR=1" << " # Do not touch this" << endl << endl <<
	  "set datafile separator \";\";" << endl << endl <<
	  "# set term postscript eps solid color;" << endl <<
	  "# set term pdfcairo solid color lw 2;" << endl <<
	  "# set term png size 800,600;" << endl <<
	  "# set term wxt size 800,600;" << endl << endl;

	string TITLE = string("\"") + "Evolution for " + m->getTitleName()
	  + " model\\n" + os->toString (true) + " - " + groupName + " - " + regionName
	  + "\"\n";

	gplot << "set multiplot title " << TITLE << endl << endl;

	double occupied_in_multiplot = 0.;

#if defined(CALLSTACK_ANALYSIS)
#define CALLSTACK_ANALYSIS_PART 0.20
	gnuplotGeneratorCallstack::generate (CALLSTACK_ANALYSIS_PART,
	  0.95-CALLSTACK_ANALYSIS_PART, gplot, fdname, ig, pcf);
	occupied_in_multiplot += CALLSTACK_ANALYSIS_PART;
#undef CALLSTACK_ANALYSIS_PART
#endif

#if defined(MEMORY_ANALYSIS)
#define CALLSTACK_REFERENCE_PART 0.25
	gnuplotGeneratorReferences::generate (CALLSTACK_REFERENCE_PART,
	  0.95-CALLSTACK_REFERENCE_PART-occupied_in_multiplot, gplot, fdname, ig,
	  variables);
	occupied_in_multiplot += CALLSTACK_REFERENCE_PART;
#undef CALLSTACK_REFERENCE_PART
#endif

	gplot << "set size 1," << 0.925-occupied_in_multiplot << ";" << endl
	      << "set origin 0,0;" << endl 
	      << "set tmargin 0; set lmargin 14; set rmargin 17;" << endl << endl;

	if (m->isY1Stacked())
	{
		gplot << endl <<
		  "set style histogram rowstacked;" << endl <<
		  "set style data histogram;" << endl <<
		  "set style fill solid 1 noborder;" << endl;
	}

	gplot << endl <<
	  "set key bottom outside center horizontal samplen 1;" << endl <<
	  "set y2range [0:*];" << endl;
	if (m->isY1Stacked())
		gplot << "set yrange [0:1];" << endl <<
		  "set x2range [0:" << nsteps << "*1./FACTOR];" << endl;
	else
		gplot << "set yrange [0:*];" << endl <<
		  "set x2range [0:*];" << endl;

	gplot <<
	  "set xtics nomirror format \"%.02f\";" << endl <<
	  "unset x2tics" << endl <<
	  "set xlabel 'Time (ms)';" << endl <<
	  "set ylabel '" << m->getY1AxisName() <<  "';" << endl <<
	  "set y2label '" << m->getY2AxisName() << "';" << endl <<
	  "set ytics nomirror;" << endl <<
	  "set y2tics nomirror format \"%g\";" << endl;

	gplot << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i << "./5.*X_LIMIT";
	gplot << ", X_LIMIT/FACTOR);" << endl;
	gplot << "set xrange [0:X_LIMIT*1./FACTOR];" << endl;

	/* If the instance-group has more than the regular 0..1 breakpoints,
	   add this into the plot */
	vector<double> brks = ig->getInterpolationBreakpoints ();
	if (brks.size () > 2)
	{
		gplot << endl;
		gplot << fixed << setprecision(8);
		for (unsigned u = 1; u < brks.size()-1; u++) /* Skip 0 and 1 */
			gplot << "set arrow from graph FACTOR*"
			  << brks[u] << ",0 to graph " << brks[u]
			  << ",1 nohead ls 0 lc rgb 'black' lw 2 front;" << endl;

		for (unsigned u = 1; u < brks.size(); u++)
		{
			double half = brks[u-1] + (brks[u]-brks[u-1])/2;
			gplot << "set label \"Phase " << u << "\" at graph FACTOR*" << half 
			  << ",0.95 rotate by -90 front textcolor rgb '#808080';" << endl;
		}
		gplot << fixed << setprecision(2);
	}
	else
		gplot << "# Unneeded phases separators, nb. breakpoints = " << brks.size() << endl << endl;

	/* Generate functions to filter the .csv according to the components of 
	   the model*/
	vector<ComponentModel*> vcm = m->getComponents();
	for (unsigned cm = 0; cm < vcm.size(); cm++)
	{
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

		string Yaxis = vcm[cm]->getPlotLocation();

		if (Yaxis == "y1" && m->isY1Stacked())
			// First grep the data we want from the CSV
			gplot << "'< grep ^\"" << regionName << ";" << numGroup << ";" << m->getName() <<
			  "_" << vcm[cm]->getName() << ";\" " << fslname << "' u (slope_" << m->getName() <<
			  "_" << vcm[cm]->getName() << "($5, strcol(3), strcol(1), $2)) ti '" << 
			  vcm[cm]->getTitleName() << "' axes x2" << Yaxis;
		else
			gplot << "'" << fslname << "' u ($4 * X_LIMIT):(slope_" << m->getName() << "_" <<
			  vcm[cm]->getName() << "($5, strcol(3), strcol(1), $2)) ti '" << 
			  vcm[cm]->getTitleName() << "' axes x1" << Yaxis  << " w lines lw 3";

		if (vcm[cm]->hasColor())
			gplot << " lc rgbcolor '" << vcm[cm]->getColor() << "'";
	}
	gplot << ";" << endl << endl
	  << "unset label;" << endl
	  << "unset arrow;" << endl << endl
	  << "unset multiplot;" << endl;

	gplot.close();

	return gname;
}

void gnuplotGenerator::gnuplot_groups (
	InstanceContainer *ic,
	const ObjectSelection *os,
	const string & prefix,
	StatisticType_t type
)
{
	string regionName = ic->getRegionName();
	InstanceSeparator *is = ic->getInstanceSeparator();
	string fname = prefix + "." + os->toString (false, "any") + ".groups.csv";
	string gname = prefix + "." + os->toString (false, "any") + "." + 
	  common::removeUnwantedChars(regionName) + ".groups.gnuplot";

	vector<string> lightcolors = { "#FF0000", "#00B000", "#0000FF", "#FF8000", "#FF0080", "#00A0A0" };
	vector<string> darkcolors  = { "#FF8080", "#80B080", "#8080FF", "#FF8080", "#FF8080", "#80A0A0" };

	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return;
	}

	gplot << fixed <<
	  "# set term postscript eps enhaced solid color" << endl <<
	  "# set term png size 800,600" << endl <<
	  "# set term wxt size 800,600;" << endl << endl <<
	  "set datafile separator \";\"" << endl <<
	  "set key bottom outside center horizontal samplen 1;" << endl <<
	  "set yrange [-1:1];" << endl <<
	  "set noytics; " << endl <<
	  "set xtics nomirror rotate by -90; " << endl <<
	  "set border 1" << endl <<
	  "set xlabel \"Time (ms)\";" << endl <<
	  "set title \"Instance groups for " << os->toString(true) << " - Region " << 
	  regionName << "\\n" << ic->numInstances() << " Instances - " << is->details() <<
	   "\"" << endl;

#define MAXLINECOLOR 6
	gplot << endl
	  << "set style line 1 pt 1 lc rgb \"" << lightcolors[0] << "\"" << endl
	  << "set style line 2 pt 2 lc rgb \"" << lightcolors[1] << "\"" << endl
	  << "set style line 3 pt 3 lc rgb \"" << lightcolors[2] << "\"" << endl
	  << "set style line 4 pt 4 lc rgb \"" << lightcolors[3] << "\"" << endl
	  << "set style line 5 pt 5 lc rgb \"" << lightcolors[4] << "\"" << endl
	  << "set style line 6 pt 6 lc rgb \"" << lightcolors[5] <<  "\"" << endl
	  << endl 
	  << "set style line 7 pt 1 lc rgb \"" << darkcolors[0] << "\"" << endl
	  << "set style line 8 pt 2 lc rgb \"" << darkcolors[1] <<  "\"" << endl
	  << "set style line 9 pt 3 lc rgb \"" << darkcolors[2] << "\"" << endl
	  << "set style line 10 pt 4 lc rgb \"" << darkcolors[3] << "\"" << endl
	  << "set style line 11 pt 5 lc rgb \"" << darkcolors[4] << "\"" << endl
	  << "set style line 12 pt 6 lc rgb \"" << darkcolors[5] << "\"" << endl
	  << endl;

	for (unsigned u = 0; u < ic->numGroups(); u++)
	{
		unsigned lstyle = (u % MAXLINECOLOR)+1;
		InstanceGroup *ig = ic->getInstanceGroup (u);
		unsigned long long s = (type == STATISTIC_MEAN) ? ig->mean() : ig->median();
		gplot << "set label \"\" at " << s << ".0/1000000.0,0 point lt " << lstyle << " ps 3 lc rgb \"" << lightcolors[lstyle-1] << "\";" << endl;
	}
	gplot << endl;

	gplot << "plot \\" << endl;
	for (unsigned u = 0; u < ic->numGroups(); u++)
	{
		unsigned lstyle = (u % MAXLINECOLOR)+1;
		InstanceGroup *ig = ic->getInstanceGroup (u);

		if (u != 0)
			gplot << ",\\" << endl;

		gplot << "'" << fname << "' using (strcol(1) eq 'u' && (strcol(2) eq '" 
		  << regionName << "' && $3 == " << u+1
		  << ") ? $4 / 1000000.0: NaN) : $0 ti '" << is->nameGroup (u)
		  << " (" << ig->numInstances() << "/" << ig->numExcludedInstances() 
		  << ")' w points ls " << lstyle << ",\\" << endl;
		gplot << "'" << fname << "' using ((strcol(1) eq 'e') && (strcol(2) eq '" 
		  << regionName 
		  << "' && $3 == " << u+1 << ") ? $4 / 1000000.0: NaN) : $0 notitle w points lc rgbcolor \"#808080\" ";
	}
	gplot << ";" << endl << endl
	  << "unset label;" << endl
	  << "unset arrow;" << endl;

	gplot.close();
}

