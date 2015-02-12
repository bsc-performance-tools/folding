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
#include "common-math.H"
#include "generate-gnuplot-references.H"
#include "generate-gnuplot-callstack.H"

#include <assert.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stack>
#include <math.h>

#include "prv-types.H"
#include "pcf-common.H"

void gnuplotGeneratorReferences::generate (
	  double verticalspace,
	  double verticalorigin,
	  ofstream & gplot,
	  const string & fileDump,
	  InstanceGroup *ig,
	  const vector<VariableInfo*> & variables,
	  UIParaverTraceConfig *pcf)
{
	assert (gplot.is_open());

	gplot << "##############################" << endl
	      << "## Addresses part " << endl
	      << "##############################" << endl << endl 
	      << "address(ret,r,g,t) = (r eq '" << ig->getRegionName()
	      << "' && g == " << ig->getNumGroup() << " && t eq 'a') ? ret : NaN;" << endl << endl
	      << "set tmargin 0; set bmargin 0; set lmargin 14; set rmargin 17;" << endl << endl;

	unsigned long long minaddress_stack, maxaddress_stack;
	unsigned long long minaddress_nonstack, maxaddress_nonstack;
	bool hassomeaddress_stack = false, hassomeaddress_nonstack = false;
#if 0
	set<Sample*> samples = ig->getAllSamples();
	set<Sample*>::iterator s;
	for (s = samples.begin(); s != samples.end(); s++)
	{
		if ((*s)->hasAddressReference() && !common::addressInStack ((*s)->getAddressReference()))
		{
			if (hassomeaddress_nonstack)
			{
				minaddress_nonstack = MIN(minaddress_nonstack, (*s)->getAddressReference());
				maxaddress_nonstack = MAX(maxaddress_nonstack, (*s)->getAddressReference());
			}
			else
			{
				maxaddress_nonstack = minaddress_nonstack = (*s)->getAddressReference();
				hassomeaddress_nonstack = true;
			}
		}
		if ((*s)->hasAddressReference() && common::addressInStack ((*s)->getAddressReference()))
		{
			if (hassomeaddress_stack)
			{
				minaddress_stack = MIN(minaddress_stack, (*s)->getAddressReference());
				maxaddress_stack = MAX(maxaddress_stack, (*s)->getAddressReference());
			}
			else
			{
				maxaddress_stack = minaddress_stack = (*s)->getAddressReference();
				hassomeaddress_stack = true;
			}
		}
	}
#endif

	map<unsigned long long, VariableInfo*> seenVariables;
	for (const auto & v : variables)
	{
		set<Sample*> samples = ig->getAllSamples();
		bool any_sample_in_variable = false;
		for (const auto s : samples)
			if (s->hasAddressReference() && v->addressInVariable (s->getAddressReference()))
			{
				any_sample_in_variable = true;
				break;
			}

		if (any_sample_in_variable)
		{
			seenVariables[v->getStartAddress()] = v;
			if (!common::addressInStack (v->getStartAddress()))
			{
				if (hassomeaddress_nonstack)
				{
					minaddress_nonstack = MIN(minaddress_nonstack, v->getStartAddress());
					maxaddress_nonstack = MAX(maxaddress_nonstack, v->getEndAddress());
				}
				else
				{
					minaddress_nonstack = v->getStartAddress();
					maxaddress_nonstack = v->getEndAddress();
					hassomeaddress_nonstack = true;
				}
			}
			else
			{
				if (hassomeaddress_stack)
				{
					minaddress_stack = MIN(minaddress_stack, v->getStartAddress());
					maxaddress_stack = MAX(maxaddress_stack, v->getEndAddress());
				}
				else
				{
					minaddress_stack = v->getStartAddress();
					maxaddress_stack = v->getEndAddress();
					hassomeaddress_stack = true;
				}
			}
		}
	}

	/* Study locality per code region for each variable */
	vector<string> stack_labels, nonstack_labels;

	vector<pair<double,double>> regions = gnuplotGeneratorCallstack::CallstackRegions (ig);
	set<Sample*> samples = ig->getAllSamples();
	for (const auto & r : regions)
	{
		for (const auto & v : seenVariables)
		{
			double min_sample_time = 0., max_sample_time = 0.;
			unsigned long long min_address = 0, max_address = 0;
			bool has_times = false;
			VariableInfo* vi = v.second;

			vector< pair< double, double> > vtmp;
			for (const auto s : samples)
				if (r.first < s->getNTime() && s->getNTime() < r.second)
					if (s->hasAddressReference())
						if (vi->addressInVariable (s->getAddressReference()))
						{
							vtmp.push_back (make_pair(s->getNTime(), s->getAddressReference()));
							if (has_times)
							{
								min_sample_time = MIN(min_sample_time, s->getNTime());
								max_sample_time = MAX(max_sample_time, s->getNTime());
								min_address = MIN(min_address, s->getAddressReference());
								max_address = MAX(max_address, s->getAddressReference());
							}
							else
							{
								min_sample_time = max_sample_time = s->getNTime();
								min_address = max_address = s->getAddressReference();
							}
							
							has_times = true;
						}

			if (vtmp.size() > 0)
			{
				double slope, intercept, correlation_coefficient;
				CommonMath::LinearRegression (vtmp, slope, intercept,
				  correlation_coefficient);

				cout << "v->getName() = " << vi->getName() << " LR slope = " << slope << " coefficient = " << correlation_coefficient << endl;

				// Does the correlation coefficient indicate that the linear regression
				// is ok?, or maybe random?
				// (i.e. correlation coefficient in [0.9 , 1.1])

				stringstream res, restmp;
				if (0.9 < correlation_coefficient && correlation_coefficient < 1.1)
				{
					map<string, InterpolationResults*> irs = ig->getInterpolated ();
					InterpolationResults *ir = irs["PAPI_TOT_INS"];
				
					double t = r.first;
					double accInstructions = 0;
					while (t < r.second)
					{
						accInstructions += ir->getSlopeAt(t) * ( ig->mean() / 1000000. );
						t += 0.001;
					}

					// cout << "add rate / acc ins = " << fabs(slope)*(max_sample_time-min_sample_time) / accInstructions << endl;

					double ratio_address_per_ins = fabs(slope)*(max_sample_time-min_sample_time) / accInstructions;

					restmp << setprecision(2) << ratio_address_per_ins;
				}
				else
				{
					/* random */
					restmp << "random";
				}

				res << "set label '" << restmp.str() << " @/ins' at second "
				  << min_sample_time + (max_sample_time-min_sample_time)/2
				  << "*FACTOR," << min_address + (max_address-min_address)/2 
				  << ". center front; # " << vi->getName();

				if (common::addressInStack (vi->getStartAddress()))
					stack_labels.push_back (res.str());
				else
					nonstack_labels.push_back (res.str());

				vtmp.clear();
			}
		}
	}

	double StackPlotProportion =
		((double)(maxaddress_stack-minaddress_stack))
			/((double)(maxaddress_nonstack-minaddress_nonstack+maxaddress_stack-minaddress_stack));

	unsigned numhexdigits_maxaddress =
	  common::numHexadecimalDigits(maxaddress_stack);

	/* Generate the header, constant part of the plot */
	gplot << fixed << setprecision(2) <<
	  "set x2range [0:1];" << endl <<
	  "set yrange [0:*];" << endl <<
	  "unset xlabel;" << endl <<
	  "unset ytics;" << endl <<
	  "unset key;" << endl;

	gplot << "set label 'Addresses referenced' at screen 0.975, screen 0.65 rotate by -90 center;" << endl <<
	  "set size 1, " << verticalspace*StackPlotProportion << ";" << endl <<
	  "set origin 0, " << verticalorigin+verticalspace*(1.-StackPlotProportion) << ";" << endl << 
	  "set border 14;" << endl <<
	  "set xrange [0:X_LIMIT*1./FACTOR];" << endl;

	gplot << "MIN_ADDRESS_STACK=" << minaddress_stack << "." << endl;
	gplot << "MAX_ADDRESS_STACK=" << maxaddress_stack << "." << endl;
	gplot << "DELTA_ADDRESS_STACK=MAX_ADDRESS_STACK-MIN_ADDRESS_STACK" << endl;
	gplot << "unset ytics;" << endl
	      << "set y2tics nomirror format '%0" << numhexdigits_maxaddress << "x' ("
	      << "MIN_ADDRESS_STACK" ;
	for (int i = 1; i <= 5; i++)
		gplot << ", MIN_ADDRESS_STACK+" << i << "*DELTA_ADDRESS_STACK/5";
	gplot << ");" << endl;

	gplot << "set y2range [MIN_ADDRESS_STACK:MAX_ADDRESS_STACK];" << endl;

	vector<string> colors;
#if 0
	colors.push_back ("#c00000");
	colors.push_back ("#00c000");
	colors.push_back ("#0000c0");
	colors.push_back ("#c0c000");
	colors.push_back ("#c000c0");
	colors.push_back ("#00c0c0");
	colors.push_back ("#c0c0c0");
	colors.push_back ("#000000");
#else
	colors.push_back ("#a0a0a0");
	colors.push_back ("#606060");
#endif
	unsigned vv = 0;
	for (const auto & v : seenVariables)
	{
		VariableInfo* vi = v.second;
		if (common::addressInStack (vi->getStartAddress()))
		{
			gplot << 
			  "set object rect from second 0, second " << vi->getStartAddress() <<
			  ". to second 1, second " << vi->getEndAddress() << ". fc rgb '" <<
			  colors[vv%(colors.size())] << "' fs solid 0.25 noborder;" << endl <<
			  "set label at second -0.01, second " <<
			  vi->getStartAddress() + vi->getSize()/2 << ". '" << vi->getName() <<
			  "' front right tc rgb '" << colors[vv%(colors.size())] <<
			  "; # Size " << vi->getEndAddress() - vi->getStartAddress() << endl;
			vv++;
		}
	}
	gplot << endl;

	for (auto const & s : stack_labels)
		gplot << s << endl;

	/* PROCESS FIRST REFERENCES TO THE STACK */

	gplot << endl
	      <<"plot\\" << endl
	      << "'" << fileDump <<"' u ($4*FACTOR):(address($5, strcol(2), $3, strcol(1))) ti '@ reference' axes x2y2 w points pt 7 ps 0.5lc rgbcolor '#ff00ff';" << endl << endl;

	/* PROCESS SECOND REFERENCES OUT OF THE STACK */

	gplot << "unset label;" << endl <<
	  "set size 1, " << verticalspace*(1.-StackPlotProportion) << ";" << endl <<
	  "set origin 0, " << verticalorigin << ";" << endl <<
	  "set border 11;" << endl;

	for (const auto & v : seenVariables)
	{
		VariableInfo* vi = v.second;
		if (!common::addressInStack (vi->getStartAddress()))
		{
			gplot << 
			  "set object rect from second 0, second " << vi->getStartAddress() <<
			  ". to second 1, second " << vi->getEndAddress() << ". fc rgb '" <<
			  colors[vv%(colors.size())] << "' fs solid 0.25 noborder;" << endl <<
			  "set label at second -0.01, second " <<
			  vi->getStartAddress() + vi->getSize()/2 << ". '" << vi->getName() <<
			  "' front right; # Size " << vi->getEndAddress() - vi->getStartAddress() << endl;
			vv++;
		}
	}
	gplot << endl;

	gplot << "MIN_ADDRESS_NONSTACK=" << minaddress_nonstack << "." << endl;
	gplot << "MAX_ADDRESS_NONSTACK=" << maxaddress_nonstack << "." << endl;
	gplot << "DELTA_ADDRESS_NONSTACK=MAX_ADDRESS_NONSTACK-MIN_ADDRESS_NONSTACK" << endl;
	gplot << "unset ytics;" << endl
	      << "set y2tics nomirror format '%0" << numhexdigits_maxaddress << "x' ("
	      << "MIN_ADDRESS_NONSTACK" ;
	for (int i = 1; i <= 5; i++)
		gplot << ", MIN_ADDRESS_NONSTACK+" << i << "*DELTA_ADDRESS_NONSTACK/5";
	gplot << ");" << endl;

	gplot << "set y2range [MIN_ADDRESS_NONSTACK:MAX_ADDRESS_NONSTACK];" << endl
	      << "unset x2tics;" << endl ;

	for (auto const & s : nonstack_labels)
		gplot << s << endl;

	gplot << endl
		  << "plot\\" << endl
	      << "'" << fileDump <<"' u ($4*FACTOR):(address($5, strcol(2), $3, strcol(1))) ti '@ reference' axes x2y2 w points pt 7 ps 0.5 lc rgbcolor '#ff00ff';" << endl << endl
	      << "unset label; unset xlabel; unset x2label; unset ylabel; unset y2label;" << endl
	      << "unset xtics; unset x2tics; unset ytics; unset y2tics;" << endl
	      << "unset xrange; unset x2range; unset yrange; unset y2range;" << endl
	      << "unset tmargin; unset bmargin; unset lmargin; unset rmargin" << endl
	      << "unset label;" << endl
	      << "unset arrow;" << endl
	      << "set border 15;" << endl
	      << "unset obj;" << endl << endl;
}