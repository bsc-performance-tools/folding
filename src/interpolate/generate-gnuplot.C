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

#include <iostream>
#include <iomanip>


void gnuplotGenerator::gnuplot_single (InstanceGroup *ig,
	const ObjectSelection *os,
	const string &prefix,
	const string &counter,
	InterpolationResults *idata,
	const string & TimeUnit)
{
	string regionName = ig->getRegion();
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

	double m = ig->mean(TimeUnit);

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
	if (ig->numExcludedSamples (counter) > 0)
	{
		gplot << "'" << fdname << "' u 4:(sampleexcluded($6, strcol(5), strcol(2), $3, strcol(1))) ti 'Excluded samples (" << ig->numExcludedSamples(counter) << ")' axes x2y1 w points pt 3 lc rgbcolor '#A0A0A0'";
		coma = true;
	}
	if (ig->numInstances())
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

string gnuplotGenerator::gnuplot_slopes (InstanceGroup *ig,
	const ObjectSelection *os,
	const string &prefix,
	bool per_instruction,
	const string & TimeUnit)
{
	string regionName = ig->getRegion();
	string groupName = ig->getGroupName();
	unsigned numGroup = ig->getNumGroup();
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

	double m = ig->mean();

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
	gplot << "# Breakpoints" << endl;
	vector<double> brks = ig->getInterpolationBreakpoints();
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

string gnuplotGenerator::gnuplot_model (InstanceGroup *ig,
	const ObjectSelection *os,
	const string & prefix,
	const Model *m,
	const string & TimeUnit)
{
	string regionName = ig->getRegion();
	string groupName = ig->getGroupName();
	unsigned numGroup = ig->getNumGroup();
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

	double me = ig->mean();

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
	vector<double> brks = ig->getInterpolationBreakpoints ();
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

string gnuplotGenerator::gnuplot_addresses (InstanceGroup *ig,
	const ObjectSelection *os,
	const string & prefix,
	const string & TimeUnit,
	unsigned long long minaddress,
	unsigned long long maxaddress,
	vector<VariableInfo*> & variables)
{
	string regionName = ig->getRegion();
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

	/* Consider variables for min & max addresses in the plot */
	vector<VariableInfo*>::iterator v;
	for (v = variables.begin(); v != variables.end(); v++)
	{
		minaddress = MIN(minaddress, (*v)->getStartAddress());
		maxaddress = MAX(maxaddress, (*v)->getEndAddress());
	}

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
	vector<double> brks = ig->getInterpolationBreakpoints ();
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
	  "' && g == 0) ? ret : NaN;" << endl << endl;

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

