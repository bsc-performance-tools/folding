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

#include <assert.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stack>

void gnuplotGenerator::gnuplot_single (
	InstanceGroup *ig,
	const ObjectSelection *os,
	const string &prefix,
	const string &counter,
	InterpolationResults *idata,
	const string & TimeUnit,
	const map<unsigned,string> & hParaverIdRoutine
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
	  "# set term pdfcairo solid color lw 2 font \",9\";" << endl <<
	  "# set term png size 800,600;" << endl << 
	  "set term wxt size 800,600;" << endl << endl;

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

#if defined(CALLSTACK_ANALYSIS)
	gplot << "set multiplot title " << TITLE << endl << endl;

	gplot << "set size 1,0.25;" << endl
	      << "set origin 0,0.65;" << endl << endl;

	gnuplot_routine_plot (gplot, fdname, ig, hParaverIdRoutine);

	gplot << "set size 1,0.7;" << endl
	      << "set origin 0,0;" << endl << endl;
#else
	gplot << "set title " << TITLE << endl << endl;
#endif

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "set key bottom outside center horizontal samplen 1;" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:1];" << endl <<
	  "set y2range [0:*];" << endl <<
	  "set ytics nomirror;" << endl <<
	  "set y2tics nomirror format \"%g\";" << endl <<
	  "set x2tics nomirror format \"%.02f\";" << endl <<
	  "set ylabel 'Normalized " << counter << "';" << endl;

	if (TimeUnit == common::DefaultTimeUnit)
		gplot << "set xlabel 'Time (in ms)';" << endl;
	else
		gplot << "set xlabel '" << TimeUnit << " (in Mevents)';" << endl;

  	gplot << "set xtics nomirror format \"%.02f\";" << endl
	  << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i << "./5.*X_LIMIT";
	gplot << ");" << endl
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
	const map<unsigned,string> & hParaverIdRoutine
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
	  "# set term pdfcairo solid color lw 2 font \",9\";" << endl <<
	  "# set term png size 800,600;" << endl <<
	  "set term wxt size 800,600;" << endl << endl;

	string TITLE = string("\"") + os->toString (true) + " - " + groupName + " - " +
	  regionName + "\"";

#if defined(CALLSTACK_ANALYSIS)
	gplot << "set multiplot title " << TITLE << endl << endl;

	gplot << "set size 1,0.25;" << endl
	      << "set origin 0,0.65;" << endl << endl;

	gnuplot_routine_plot (gplot, fdname, ig, hParaverIdRoutine);

	gplot << "set size 1,0.7;" << endl
	      << "set origin 0,0;" << endl << endl;
#else
	gplot << "set title " << TITLE << endl << endl;
#endif

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "set key bottom outside center horizontal samplen 1;" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:*];" << endl <<
	  "set y2range [0:*];" << endl <<
	  "set x2tics nomirror format \"%.02f\";" << endl;

	if (TimeUnit == common::DefaultTimeUnit)
	{
		gplot << "set xlabel 'Time (in ms)';" << endl
	  	  << "set xtics nomirror format \"%.02f\";" << endl
		  << "set xtics ( 0.0 ";
		for (int i = 1; i <= 5; i++)
			gplot << ", " << i << "./5.*X_LIMIT";
		gplot << ");" << endl
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
		  "set ytics nomirror;" << endl <<
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
					  << "($5, strcol(3), strcol(1), $2)) ti '" << (*it).first
					  << "/ins' axes x2y1 w lines lw 3";
				else
					gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti '" << (*it).first
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
	const string & TimeUnit,
	const map<unsigned,string> & hParaverIdRoutine
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
	  "# set term pdfcairo solid color lw 2 font \",9\";" << endl <<
	  "# set term png size 800,600;" << endl <<
	  "set term wxt size 800,600;" << endl << endl;

	string TITLE = string("\"") + "Evolution for " + m->getTitleName()
	  + " model\\n" + os->toString (true) + " - " + groupName + " - " + regionName
	  + "\n";

#if defined(CALLSTACK_ANALYSIS)
	gplot << "set multiplot title " << TITLE << endl << endl;

	gplot << "set size 1,0.25;" << endl
	      << "set origin 0,0.65;" << endl << endl;

	gnuplot_routine_plot (gplot, fdname, ig, hParaverIdRoutine);

	gplot << "set size 1,0.7;" << endl
	      << "set origin 0,0;" << endl << endl;
#else
	gplot << "set title " << TITLE << endl << endl;
#endif

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
	  "set xlabel 'Time (in ms)';" << endl <<
	  "set ylabel '" << m->getY1AxisName() <<  "';" << endl <<
	  "set y2label '" << m->getY2AxisName() << "';" << endl <<
	  "set ytics nomirror;" << endl <<
	  "set y2tics nomirror format \"%g\";" << endl;

	gplot << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i << "./5.*X_LIMIT";
	gplot << ");" << endl;
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

string gnuplotGenerator::gnuplot_addresses_cost (
	InstanceGroup *ig,
	const ObjectSelection *os,
	const string & prefix,
	const string & TimeUnit,
	vector<VariableInfo*> & variables,
	const map<unsigned,string> & hParaverIdRoutine
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
	gname += ".addresses_cost.gnuplot";

	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return "";
	}

	double m = ig->mean();

	/** FIRST PROCESS REFERENCES IN STACK! **/

	/* Consider variables for min & max addresses in the plot */
	unsigned long long minaddress = 0, maxaddress = 0, maxcycles = 0;
	bool hassomeaddress = false;
	set<Sample*> samples = ig->getAllSamples();
	set<Sample*>::iterator s;
	for (s = samples.begin(); s != samples.end(); s++)
		if ((*s)->hasAddressReference() && (*s)->hasAddressReferenceInStack())
		{
			if (hassomeaddress)
			{
				minaddress = MIN(minaddress, (*s)->getAddressReference());
				maxaddress = MAX(maxaddress, (*s)->getAddressReference());
				maxcycles  = MAX(maxcycles,  (*s)->getAddressReference_Cycles_Cost());
			}
			else
			{
				maxaddress = minaddress = (*s)->getAddressReference();
				hassomeaddress = true;
			}
		}

	unsigned numhexdigits_maxaddress =
	  common::numHexadecimalDigits(maxaddress);

	/* Generate the pre-info */
	gplot << fixed << setprecision(2) <<
	  "X_LIMIT=" << m / 1000000 << " # Do not touch this" << endl <<
	  "FACTOR=1" << " # Do not touch this" << endl << endl;

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "# set term postscript eps solid color;" << endl <<
	  "# set term pdfcairo solid color lw 2 font \",9\";" << endl <<
	  "# set term png size 800,600;" << endl <<
	  "# set term x11 size 800,600;" << endl <<
	  "set term wxt size 800,600;" << endl << endl <<
	  "set multiplot layout 2,1" << endl << 
	  "set datafile separator \";\";" << endl <<
	  "set key bottom outside center horizontal samplen 1;" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:*];" << endl <<
	  "set xtics nomirror format \"%.02f\";" << endl <<
	  "set x2tics nomirror format \"%.02f\";" << endl <<
	  "set xlabel 'Time (in ms)';" << endl <<
	  "set ylabel 'Performance counter rate (in Mevents/s)';" << endl <<
	  "set ytics nomirror;" << endl <<
	  "set y2label 'Addresses referenced';" << endl;

	gplot << "set title \"" << os->toString (true) << " - " << groupName 
	  <<  " - " << regionName << "\"" <<endl;

	gplot << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i << "./5.*X_LIMIT";
	gplot << ");" << endl
	  << "set xrange [0:X_LIMIT*1./FACTOR];" << endl;

	gplot << "set y2tics nomirror format '%0" << numhexdigits_maxaddress << "x' (" << minaddress;
	for (int i = 1; i <= 5; i++)
		gplot << ", " << minaddress+i*((maxaddress-minaddress)/5);
	gplot << ");" << endl
	  << "set y2range [" << minaddress << ":" << maxaddress << "];" << endl;

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
		gplot << endl << "# Unneeded phases separators, nb. breakpoints = " << brks.size() << endl;

	/* Generate functions to filter the .csv */
	map<string, InterpolationResults*>::iterator it;
	map<string, InterpolationResults*> interpolated = ig->getInterpolated ();
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

	gplot << endl << "MAX_COST_GRADIENT = " << maxcycles << ";" << endl <<
	  "address_COST_GRADIENT(x) = (int(((MAX_COST_GRADIENT-x)/MAX_COST_GRADIENT)*65535)&0xff00) + (x/MAX_COST_GRADIENT)*255;" << endl;
	gplot << "address_COST_CYCLES(ret,r,g,t) = (r eq '" << regionName <<
	  "' && g == " << numGroup << " && t eq 'a') ? ret : NaN;" << endl << endl;

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
				gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y1 w lines lw 3";
			else
				gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti '" << (*it).first
				      << "' axes x2y1 w lines lw 3";

			count++;
		}
	}
	gplot << ",\\" << endl << 
	  "'" << fdname << "' u ($4*FACTOR):(address_COST_CYCLES($5, strcol(2), $3, strcol(1))):(address_COST_GRADIENT($7)) ti 'Reference cost (max="
	  << maxcycles << "cycles)' axes x2y2 lc rgb variable;" << endl;

	/** SECOND PROCESS REFERENCES OUT OF STACK! **/

	hassomeaddress = false;
	for (s = samples.begin(); s != samples.end(); s++)
		if ((*s)->hasAddressReference() && !(*s)->hasAddressReferenceInStack())
		{
			if (hassomeaddress)
			{
				minaddress = MIN(minaddress, (*s)->getAddressReference());
				maxaddress = MAX(maxaddress, (*s)->getAddressReference());
				maxcycles  = MAX(maxcycles,  (*s)->getAddressReference_Cycles_Cost());
			}
			else
			{
				maxaddress = minaddress = (*s)->getAddressReference();
				hassomeaddress = true;
			}
		}
	vector<VariableInfo*>::iterator v;
	for (v = variables.begin(); v != variables.end(); v++)
	{
		minaddress = MIN(minaddress, (*v)->getStartAddress());
		maxaddress = MAX(maxaddress, (*v)->getEndAddress());
	}

	gplot << endl << endl
	  << "set y2tics nomirror format '%0" << numhexdigits_maxaddress << "x' (" << minaddress;
	for (int i = 1; i <= 5; i++)
		gplot << ", " << minaddress+i*((maxaddress-minaddress)/5);
	gplot << ");" << endl
	  << "set y2range [" << minaddress << ":" << maxaddress << "];" << endl << endl;

	vector<string> colors;
	colors.push_back ("#c00000");
	colors.push_back ("#00c000");
	colors.push_back ("#0000c0");
	colors.push_back ("#c0c000");
	colors.push_back ("#c000c0");
	colors.push_back ("#00c0c0");
	colors.push_back ("#c0c0c0");
	colors.push_back ("#000000");
	unsigned vv;
	for (vv = 0, v = variables.begin(); v != variables.end(); v++, vv++)
	{
		gplot << 
		  "set object rect from second 0, second " << (*v)->getStartAddress() <<
		  " to second 1, second " << (*v)->getEndAddress() << " fc rgb '" <<
		  colors[vv%(colors.size())] << "' fs solid 0.25;" << endl <<
		  "set label at second 0.975, second " <<
		  (*v)->getStartAddress() + (*v)->getSize()/2 << " '" << (*v)->getName() <<
		  "' front center;" << endl;
	}
	gplot << endl;

	/* Generate the plot command */
	count = 0;
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
				gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y1 w lines lw 3";
			else
				gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti '" << (*it).first
				      << "' axes x2y1 w lines lw 3";

			count++;
		}
	}
	gplot << ",\\" << endl << 
	  "'" << fdname << "' u ($4*FACTOR):(address_COST_CYCLES($5, strcol(2), $3, strcol(1))):(address_COST_GRADIENT($7)) ti 'Reference cost (max="
	  << maxcycles << "cycles)' axes x2y2 lc rgb variable;" << endl;

	gplot << endl
	  << "unset label;" << endl
	  << "unset arrow;" << endl
	  << "unset multiplot;" << endl;

	gplot.close();

	return gname;
}

string gnuplotGenerator::gnuplot_addresses (
	InstanceGroup *ig,
	const ObjectSelection *os,
	const string & prefix,
	const string & TimeUnit,
	vector<VariableInfo*> & variables,
	const map<unsigned,string> & hParaverIdRoutine
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
	gname += ".addresses.gnuplot";

	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return "";
	}

	double m = ig->mean();

	/* PROCESS FIRST REFERENCES TO THE STACK */

	unsigned long long minaddress = 0, maxaddress = 0, maxcycles = 0;
	bool hassomeaddress = false;
	set<Sample*> samples = ig->getAllSamples();
	set<Sample*>::iterator s;
	for (s = samples.begin(); s != samples.end(); s++)
		if ((*s)->hasAddressReference() && (*s)->hasAddressReferenceInStack())
		{
			if (hassomeaddress)
			{
				minaddress = MIN(minaddress, (*s)->getAddressReference());
				maxaddress = MAX(maxaddress, (*s)->getAddressReference());
				maxcycles  = MAX(maxcycles,  (*s)->getAddressReference_Cycles_Cost());
			}
			else
			{
				maxaddress = minaddress = (*s)->getAddressReference();
				hassomeaddress = true;
			}
		}

	unsigned numhexdigits_maxaddress =
	  common::numHexadecimalDigits(maxaddress);

	/* Generate the pre-info */
	gplot << fixed << setprecision(2) <<
	  "X_LIMIT=" << m / 1000000 << " # Do not touch this" << endl <<
	  "FACTOR=1" << " # Do not touch this" << endl << endl;

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "# set term postscript eps solid color;" << endl <<
	  "# set term pdfcairo solid color lw 2 font \",9\";" << endl <<
	  "# set term png size 800,600;" << endl <<
	  "# set term x11 size 800,600;" << endl <<
	  "set term wxt size 800,600;" << endl << endl <<
	  "set multiplot layout 2,1" << endl << 
	  "set datafile separator \";\";" << endl <<
	  "set key bottom outside center horizontal samplen 1;" << endl <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:*];" << endl <<
	  "set xtics nomirror format \"%.02f\";" << endl <<
	  "set x2tics nomirror format \"%.02f\";" << endl <<
	  "set xlabel 'Time (in ms)';" << endl <<
	  "set ylabel 'Performance counter rate (in Mevents/s)';" << endl <<
	  "set ytics nomirror;" << endl <<
	  "set y2label 'Addresses referenced';" << endl;

	gplot << "set title \"" << os->toString (true) << " - " << groupName 
	  <<  " - " << regionName << "\"" << endl;

	gplot << "set xtics ( 0.0 ";
	for (int i = 1; i <= 5; i++)
		gplot << ", " << i << "./5.*X_LIMIT";
	gplot << ");" << endl
	  << "set xrange [0:X_LIMIT*1./FACTOR];" << endl;

	gplot << "set y2tics nomirror format '%0" << numhexdigits_maxaddress << "x' (" << minaddress;
	for (int i = 1; i <= 5; i++)
		gplot << ", " << minaddress+i*((maxaddress-minaddress)/5);
	gplot << ");" << endl
	  << "set y2range [" << minaddress << ":" << maxaddress << "];" << endl;

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
		gplot << endl << "# Unneeded phases separators, nb. breakpoints = " << brks.size() << endl;

	/* Generate functions to filter the .csv */
	map<string, InterpolationResults*>::iterator it;
	map<string, InterpolationResults*> interpolated = ig->getInterpolated ();
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

	gplot << "address_L1(ret,w,r,g,t) = (w == 1 && r eq '" << regionName <<
	  "' && g == " << numGroup << " && t eq 'a') ? ret : NaN;" << endl;
	gplot << "address_LFB(ret,w,r,g,t) = (w == 2 && r eq '" << regionName <<
	  "' && g == " << numGroup << " && t eq 'a') ? ret : NaN;" << endl;
	gplot << "address_L2(ret,w,r,g,t) = (w == 3 && r eq '" << regionName <<
	  "' && g == " << numGroup << " && t eq 'a') ? ret : NaN;" << endl;
	gplot << "address_L3(ret,w,r,g,t) = (w == 4 && r eq '" << regionName <<
	  "' && g == " << numGroup << " && t eq 'a') ? ret : NaN;" << endl;
	gplot << "address_RCACHE(ret,w,r,g,t) = (w == 5 && r eq '" << regionName <<
	  "' && g == " << numGroup << " && t eq 'a') ? ret : NaN;" << endl;
	gplot << "address_DRAM(ret,w,r,g,t) = (w == 6 && r eq '" << regionName <<
	  "' && g == " << numGroup << " && t eq 'a') ? ret : NaN;" << endl;
	gplot << "address_OTHER(ret,w,r,g,t) = (w == 0 && r eq '" << regionName <<
	  "' && g == " << numGroup << " && t eq 'a') ? ret : NaN;" << endl << endl;

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
				gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y1 w lines lw 3";
			else
				gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti '" << (*it).first
				      << "' axes x2y1 w lines lw 3";

			count++;
		}
	}

	gplot << ",\\" << endl << 
	  "'" << fdname <<"' u ($4*FACTOR):(address_L1($5, $6, strcol(2), $3, strcol(1))) ti 'L1 reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_LFB($5, $6, strcol(2), $3, strcol(1))) ti 'LFB reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_L2($5, $6, strcol(2), $3, strcol(1))) ti 'L2 reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_L3($5, $6, strcol(2), $3, strcol(1))) ti 'L3 reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_RCACHE($5, $6, strcol(2), $3, strcol(1))) ti 'RCache reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_DRAM($5, $6, strcol(2), $3, strcol(1))) ti 'DRAM reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_OTHER($5, $6, strcol(2), $3, strcol(1))) ti 'Other reference' axes x2y2 w points pt 3;" << endl;

	gplot << endl << endl;

	/* PROCESS SECOND REFERENCES OUT OF THE STACK */

	hassomeaddress = false;
	for (s = samples.begin(); s != samples.end(); s++)
		if ((*s)->hasAddressReference() && !(*s)->hasAddressReferenceInStack())
		{
			if (hassomeaddress)
			{
				minaddress = MIN(minaddress, (*s)->getAddressReference());
				maxaddress = MAX(maxaddress, (*s)->getAddressReference());
				maxcycles  = MAX(maxcycles,  (*s)->getAddressReference_Cycles_Cost());
			}
			else
			{
				maxaddress = minaddress = (*s)->getAddressReference();
				hassomeaddress = true;
			}
		}
	vector<VariableInfo*>::iterator v;
	for (v = variables.begin(); v != variables.end(); v++)
	{
		minaddress = MIN(minaddress, (*v)->getStartAddress());
		maxaddress = MAX(maxaddress, (*v)->getEndAddress());
	}

	gplot << "set y2tics nomirror format '%0" << numhexdigits_maxaddress << "x' (" << minaddress;
	for (int i = 1; i <= 5; i++)
		gplot << ", " << minaddress+i*((maxaddress-minaddress)/5);
	gplot << ");" << endl
	  << "set y2range [" << minaddress << ":" << maxaddress << "];" << endl;

	vector<string> colors;
	colors.push_back ("#c00000");
	colors.push_back ("#00c000");
	colors.push_back ("#0000c0");
	colors.push_back ("#c0c000");
	colors.push_back ("#c000c0");
	colors.push_back ("#00c0c0");
	colors.push_back ("#c0c0c0");
	colors.push_back ("#000000");
	unsigned vv;
	for (vv = 0, v = variables.begin(); v != variables.end(); v++, vv++)
	{
		gplot << 
		  "set object rect from second 0, second " << (*v)->getStartAddress() <<
		  " to second 1, second " << (*v)->getEndAddress() << " fc rgb '" <<
		  colors[vv%(colors.size())] << "' fs solid 0.25;" << endl <<
		  "set label at second 0.975, second " <<
		  (*v)->getStartAddress() + (*v)->getSize()/2 << " '" << (*v)->getName() <<
		  "' front center;" << endl << endl;
	}

	count = 0;
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
				gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y1 w lines lw 3";
			else
				gplot << "'" << fslname << "' u ($4*FACTOR):(slope_" << counter_gnuplot
					  << "($5, strcol(3), strcol(1), $2)) ti '" << (*it).first
				      << "' axes x2y1 w lines lw 3";

			count++;
		}
	}

	gplot << ",\\" << endl << 
	  "'" << fdname <<"' u ($4*FACTOR):(address_L1($5, $6, strcol(2), $3, strcol(1))) ti 'L1 reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_LFB($5, $6, strcol(2), $3, strcol(1))) ti 'LFB reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_L2($5, $6, strcol(2), $3, strcol(1))) ti 'L2 reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_L3($5, $6, strcol(2), $3, strcol(1))) ti 'L3 reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_RCACHE($5, $6, strcol(2), $3, strcol(1))) ti 'RCache reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_DRAM($5, $6, strcol(2), $3, strcol(1))) ti 'DRAM reference' axes x2y2 w points pt 3,\\" << endl <<
	  "'" << fdname << "' u ($4*FACTOR):(address_OTHER($5, $6, strcol(2), $3, strcol(1))) ti 'Other reference' axes x2y2 w points pt 3;" << endl;

	gplot << endl << endl
	  << "unset label;" << endl
	  << "unset arrow;" << endl
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

	vector<string> lightcolors, darkcolors;
	lightcolors.push_back ("#FF0000"); darkcolors.push_back ("#FF8080");
	lightcolors.push_back ("#00B000"); darkcolors.push_back ("#80B080");
	lightcolors.push_back ("#0000FF"); darkcolors.push_back ("#8080FF");
	lightcolors.push_back ("#FF8000"); darkcolors.push_back ("#FF8080");
	lightcolors.push_back ("#FF0080"); darkcolors.push_back ("#FF8080");
	lightcolors.push_back ("#00A0A0"); darkcolors.push_back ("#80A0A0");

	ofstream gplot (gname.c_str());

	if (!gplot.is_open())
	{
		cerr << "Failed to create " << gname << endl;
		return;
	}

	gplot << fixed <<
	  "# set term postscript eps enhaced solid color" << endl <<
	  "# set term png size 800,600" << endl <<
	  "set term wxt size 800,600;" << endl << endl <<
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
		gplot << "set label \"\" at " << s << ".0/1000000.0,0 point lt " << lstyle << " ps 2 lc rgb \"" << lightcolors[lstyle-1] << "\";" << endl;
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
		  << "' && $3 == " << u+1 << ") ? $4 / 1000000.0: NaN) : $0 notitle w points ls " 
		  << lstyle+MAXLINECOLOR;
	}
	gplot << ";" << endl << endl
	  << "unset label;" << endl
	  << "unset arrow;" << endl;

	gplot.close();
}


void gnuplotGenerator::gnuplot_routine_plot (
	ofstream & gplot,
	const string & fileDump,
	InstanceGroup *ig,
	const map<unsigned,string> & hParaverIdRoutine
)
{
	assert (gplot.is_open());

	if (!ig->hasPreparedCallstacks())
		return;

	const vector<CallstackProcessor_Result*> routines = ig->getPreparedCallstacks();

	gplot << "set xlabel \"ghost\" tc rgbcolor \"white\";" << endl
	      << "set ylabel \"ghost\" tc rgbcolor \"white\";" << endl
	      << "set y2label \"Code line\";" << endl
	      << "set label \"bottom\" at second 1.005, first 0;" << endl
	      << "set label \"top\"    at second 1.005, first 1;" << endl
	      << "set xrange [0:X_LIMIT*1./FACTOR];" << endl
	      << "set x2range [0:1];" << endl
	      << "set yrange [0:1];" << endl
	      << "set y2range [0:*] reverse;" << endl
	      << "set ytics tc rgbcolor \"white\" (0.001) format \"%0.2f\";" << endl
	      << "set y2tics 100 tc rgbcolor \"white\" format \"0000\";" << endl
	      << "unset xtics;" << endl
	      << "unset x2tics;" << endl
	      << endl;

	vector<CallstackProcessor_Result*>::const_iterator it = routines.cbegin();
	double last = 0.;
	vector<string> bgcolors;
	bgcolors.push_back (string("#ff0000"));
	bgcolors.push_back (string("#00ff00"));
	bgcolors.push_back (string("#0000ff"));
	bgcolors.push_back (string("#ffa000"));
	bgcolors.push_back (string("#00ffff"));
	bgcolors.push_back (string("#606060"));

#define X_WIDTH_THRESHOLD 0.01

	gplot << fixed << setprecision(3) << endl;

	map<string, double> routines_time;
	map<unsigned, string> routines_colors;
	unsigned idx = 0;
	stack<unsigned> routine_stack;
	for (const auto & r : routines)
	{
		double duration = r->getNTime() - last;
		unsigned top = routine_stack.empty()?0:routine_stack.top();

		string routine;
		if (hParaverIdRoutine.count (top) > 0)
			routine = hParaverIdRoutine.at(top);
		else
			routine = "Unknown";

		if (duration >= X_WIDTH_THRESHOLD)
		{
			string color;
			if (routines_colors.count (top) == 0)
			{
				color = bgcolors[idx];
				idx = (idx+1)%bgcolors.size();
			}
			else
				color = routines_colors.at (top);

			gplot << "set obj rect from graph " << last << ", graph 0 to graph "
			      << r->getNTime() << ", graph 1 "
			      << "fs transparent solid 0.33 noborder fc rgbcolor '" << color
			      << "' behind # Routine: " << routine << " "
			      << duration * 100.f << "%" << endl;

			routines_colors[top] = color;
		}

		if (!routine_stack.empty())
		{
			if (routines_time.count (routine) == 0)
				routines_time[routine] = duration*100.;
			else
				routines_time[routine] += duration*100.;
		}

		if (r->getCaller() != 0)
			routine_stack.push (r->getCaller());
		else
			routine_stack.pop ();

		last = r->getNTime();
	}

	gplot << endl;

	for (const auto rt : routines_time)
		gplot << "# Summary for routine " << rt.first << " " << rt.second << "%" << endl;

	gplot << endl;

	last = 0.;
	routine_stack = stack<unsigned>();
	for (const auto & r : routines)
	{
		double tbegin = last;
		double tend = r->getNTime();
		double middle = tbegin + (tend-tbegin)/2;

		if (tend - tbegin >= X_WIDTH_THRESHOLD)
		{
			if (!routine_stack.empty())
			{
				unsigned top = routine_stack.top();
				if (hParaverIdRoutine.count (top) > 0)
					gplot << "set label center \"" << hParaverIdRoutine.at(top);
				else
					gplot << "set label center \"Unknown routine " << top;

				gplot << "\" at second " << middle << ", first 0.5 rotate by 90 tc rgbcolor 'black' front" << endl;
			}
		}

		if (r->getCaller() != 0)
			routine_stack.push (r->getCaller());
		else
			routine_stack.pop ();

		last = r->getNTime();
	}

	gplot << fixed << setprecision(2) << endl;

	gplot << endl
	      << "samplecls(ret,r,g,t) = (r eq '" << ig->getRegionName() << "' && g == " << ig->getNumGroup() << "  && t eq 'cl') ? ret : NaN;" << endl << endl
	      << "plot \"" << fileDump << "\" u 4:(samplecls($5,strcol(2),$3,strcol(1))) with points axes x2y2 ti '' lc rgbcolor '#ff2090' pt 7 ps 0.5;" << endl
	      << endl
	      << "unset xlabel;" << endl
	      << "unset ylabel;" << endl
	      << "unset y2label;" << endl
	      << "unset ytics;" << endl
	      << "unset y2tics;" << endl
	      << "set y2tics autofreq;" << endl
	      << "unset label;" << endl
	      << "unset arrow;" << endl
	      << "unset xrange;" << endl
	      << "unset x2range;" << endl
	      << "unset yrange;" << endl
	      << "unset y2range;" << endl
	      << endl;
}

