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
#include "generate-gnuplot-callstack.H"

#include <assert.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stack>

#include "prv-types.H"
#include "pcf-common.H"
#include "prv-colors.H"

string gnuplotGeneratorCallstack::getStackStringDepth (unsigned depth,
	stack<CodeRefTriplet> scrt, UIParaverTraceConfig *pcf)
{
	assert (depth > 0);

	string res;
	unsigned line;
	string file;
	string routine;
	try
	{ routine = pcf->getEventValue (EXTRAE_SAMPLE_CALLER, scrt.top().getCaller()); }
	catch (...)
	{ }
	pcfcommon::lookForCallerLineInfo (pcf, scrt.top().getCallerLine(), file, line);
	depth--;
	scrt.pop();

	stringstream ss;
	ss << line;
	res = routine + "\\n[" + ss.str() + "]";

	while (depth > 0 && !scrt.empty())
	{
		try
		{ routine = pcf->getEventValue (EXTRAE_SAMPLE_CALLER, scrt.top().getCaller()); }
		catch (...)
		{ }

		res = routine + " >\\n" + res;

		depth--;
		scrt.pop();
	}

	return res;
}

void gnuplotGeneratorCallstack::generate (
	double verticalsize,
	double verticalorigin,
	ofstream & gplot,
	const string & fileDump,
	InstanceGroup *ig,
	UIParaverTraceConfig *pcf
)
{
	assert (gplot.is_open());

	PRVcolors colors;

	const vector<CallstackProcessor_Result*> routines = ig->getPreparedCallstacks();

	gplot << "##############################" << endl
	      << "## Routines part " << endl
	      << "##############################" << endl << endl
	      << "samplecls(ret,r,g,t) = (r eq '" << ig->getRegionName()
		    << "' && g == " << ig->getNumGroup() << "  && t eq 'cl') ? ret : NaN;" << endl << endl
	      << "set size 1," << verticalsize << ";" << endl
	      << "set origin 0," << verticalorigin << ";" << endl << endl
	      << "set bmargin 0; set lmargin 14; set rmargin 17;" << endl << endl
	      << "set xlabel \"ghost\" tc rgbcolor \"white\";" << endl
	      << "set ylabel \"ghost\" tc rgbcolor \"white\";" << endl
	      << "#set y2label \"Code line\";" << endl
	      << "set label 'Code line' at screen 0.975, screen 0.8+(0.175/2) rotate by -90 center;" << endl
	      << "set label \"bottom\" at second 1.005, first 0;" << endl
	      << "set label \"top\"    at second 1.005, first 1;" << endl
	      << "set xrange [0:X_LIMIT*1./FACTOR];" << endl
	      << "set x2range [0:1];" << endl
	      << "set yrange [0:1];" << endl;
	if (ig->hasPreparedCallstacks())
		gplot << "set y2range [0:*] reverse;" << endl;
	else
		gplot << "set y2range [0:1] reverse; " << endl; /* gnuplot will not calculate the * */
	gplot << "set ytics tc rgbcolor \"white\" (0.001) format \"%0.2f\";" << endl
	      << "set y2tics 100 tc rgbcolor \"white\" format \"0000\";" << endl
	      << "unset xtics;" << endl
	      << "unset x2tics;" << endl
	      << endl;

	double last = 0.;

#define X_WIDTH_THRESHOLD 0.025

	gplot << fixed << setprecision(3);

	map<string, double> routines_time;
	stack<CodeRefTriplet> routine_stack;
	for (const auto & r : routines)
	{
		double duration = r->getNTime() - last;
		unsigned top = routine_stack.empty()?0:routine_stack.top().getCaller();

		string routine = "Unknown";
		try
		{ routine = pcf->getEventValue (EXTRAE_SAMPLE_CALLER, top); }
		catch (...)
		{ }

		if (duration >= X_WIDTH_THRESHOLD)
		{
			string color = colors.getString (top);
			gplot << "set obj rect from graph " << last << "*FACTOR, graph 0 to graph "
			      << r->getNTime() << "*FACTOR, graph 1 "
			      << "fs transparent solid 0.50 noborder fc rgbcolor '#" << color
			      << "' behind # Routine: " << routine << " "
			      << duration * 100.f << "%" << endl;
		}

		if (!routine_stack.empty())
		{
			if (routines_time.count (routine) == 0)
				routines_time[routine] = duration*100.;
			else
				routines_time[routine] += duration*100.;
		}

		if (r->getCodeRef().getCaller() != 0)
			routine_stack.push (r->getCodeRef());
		else
			routine_stack.pop ();

		last = r->getNTime();
	}

	if (routines_time.size() > 0)
	{
		gplot << endl;
		for (const auto rt : routines_time)
			gplot << "# Summary for routine " << rt.first << " " << rt.second << "%" << endl;
		gplot << endl;
	}

	last = 0.;
	routine_stack = stack<CodeRefTriplet>();
	for (const auto & r : routines)
	{
		double tbegin = last;
		double tend = r->getNTime();
		double middle = tbegin + (tend-tbegin)/2;

		if (tend - tbegin >= X_WIDTH_THRESHOLD)
		{
			if (!routine_stack.empty())
			{
				string routine = getStackStringDepth (3, routine_stack, pcf);

				gplot << "set label center \"" << routine
				      << "\" at second " << middle
				      << "*FACTOR, graph 0.5 rotate by 90 tc rgbcolor 'black' front" << endl;
			}
		}

		if (r->getCodeRef().getCaller() != 0)
			routine_stack.push (r->getCodeRef());
		else
			routine_stack.pop ();

		last = r->getNTime();
	}

	gplot << fixed << setprecision(2);

	gplot << endl
	      << "plot \"" << fileDump << "\" u ($4*FACTOR):(samplecls($5,strcol(2),$3,strcol(1))) with points axes x2y2 ti '' lc rgbcolor '#ff2090' pt 7 ps 0.5;" << endl
	      << endl
	      << "unset label; unset xlabel; unset x2label; unset ylabel; unset y2label;" << endl
	      << "unset xtics; unset x2tics; unset ytics; unset y2tics; set y2tics autofreq;" << endl
          << "unset xrange; unset x2range; unset yrange; unset y2range;" << endl
		  << "unset tmargin; unset bmargin; unset lmargin; unset rmargin" << endl
	      << "unset label;" << endl
	      << "unset arrow;" << endl
		  << "unset obj;" << endl
	      << endl;
}

vector<pair<double,double>> gnuplotGeneratorCallstack::CallstackRegions (
	InstanceGroup *ig
)
{
	vector<pair<double,double>> result;
	double last = 0.;
	for (const auto & r : ig->getPreparedCallstacks())
	{
		double tend = r->getNTime();
		if (tend - last >= X_WIDTH_THRESHOLD)
		{
			pair<double, double> p = make_pair (last, tend);
			result.push_back (p);
		}
		last = r->getNTime();
	}
	return result;
}

