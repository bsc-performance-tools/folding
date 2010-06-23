/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  MPItrace                                 *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *                                                             ___           *
 *   +---------+     http:// www.cepba.upc.edu/tools_i.htm    /  __          *
 *   |    o//o |     http:// www.bsc.es                      /  /  _____     *
 *   |   o//o  |                                            /  /  /     \    *
 *   |  o//o   |     E-mail: cepbatools@cepba.upc.edu      (  (  ( B S C )   *
 *   | o//o    |     Phone:          +34-93-401 71 78       \  \  \_____/    *
 *   +---------+     Fax:            +34-93-401 25 77        \  \__          *
 *    C E P B A                                               \___           *
 *                                                                           *
 * This software is subject to the terms of the CEPBA/BSC license agreement. *
 *      You must accept the terms of this license to use this software.      *
 *                                 ---------                                 *
 *                European Center for Parallelism of Barcelona               *
 *                      Barcelona Supercomputing Center                      *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>
#include <iomanip>

#include "UIParaverTraceConfig.h"
#include "kriger_wrapper.h"
#include "generate-gnuplot.H"
#include "region-analyzer.H"
#include "common.H"
#include "point.H"

#define MAX(a,b) ((a)>(b))?(a):(b)

#define MAX_REGIONS 1024

using namespace std;

class Sample
{
	public:
	string CounterID;
	unsigned Region;
	unsigned Phase;
	float Time;
	float CounterValue;
	unsigned long long DeTime;
	unsigned long long DeCounterValue;

	unsigned iteration;
};

bool FilterMinDuration = false;
double MinDuration;

unsigned MaxIteration = 0;
bool hasMaxIteration = false;

string TraceToFeed;
bool feedTraceRegion = false;
bool feedTraceTimes = false;
unsigned long long feedTraceRegion_Type;
unsigned long long feedTraceRegion_Value;
unsigned long long feedTraceFoldType_Value;
unsigned long long feedTraceTimes_Begin, feedTraceTimes_End;

unsigned numRegions = 0;
string nameRegion[MAX_REGIONS];
int countRegion[MAX_REGIONS];
double meanRegion[MAX_REGIONS], meanRegion_tot_ins[MAX_REGIONS];
double sigmaRegion[MAX_REGIONS], sigmaRegion_tot_ins[MAX_REGIONS];
double NumOfSigmaTimes;

bool option_doLineFolding = true;
int option_InterpolationErrorLevel = 10;
bool removeOutliers = false;
bool SeparateValues = true;
vector<string> wantedCounters;
list<GNUPLOTinfo*> GNUPLOT;

bool generateGNUPLOTfiles = false;

unsigned TranslateRegion (string &RegionName)
{
	/* Look for region in the translation table */
	for (unsigned i = 0; i < numRegions; i++)
		if (nameRegion[i] == RegionName)
			return i;

	/* If it can't be find, add it */
	unsigned result = numRegions;
	nameRegion[result] = RegionName;
	numRegions++;

	if (numRegions >= MAX_REGIONS)
	{
		cerr << "Reached MAX_REGIONS!" << endl;
		exit (-1);
	}

	return result;
}

void FillData (ifstream &file, bool any_region, vector<Sample> &vsamples,
	vector<Point> &accumulatedCounterPoints)
{
	unsigned long long lastRegion = 0;
	unsigned long long lastDuration = 0;
	bool inRegion = false;
	bool Outlier = false;
	char type;

	while (true)
	{
		file >> type;
		if (file.eof())
			break;

		if (type == 'D')
		{
			string strRegion;
			file >> strRegion;
			TranslateRegion (strRegion);
		}
		else if (type == 'T')
		{
			string strRegion;
			unsigned long long Duration;

			file >> strRegion;
			file >> Duration;

			lastDuration = Duration;

			lastRegion = TranslateRegion (strRegion);
			inRegion = true;

			if (removeOutliers && inRegion)
				Outlier = fabs (meanRegion[any_region?0:lastRegion] - Duration) > NumOfSigmaTimes*sigmaRegion[any_region?0:lastRegion];

#if defined(DEBUG)
			cout << "DURATION " << Duration << " REGION (" << strRegion << ")= "<< lastRegion << (Outlier?" is":" is not") << " an outlier " << endl;
			cout << "MEAN_REGION = " << meanRegion[any_region?0:lastRegion] << " ABS = " << fabs (meanRegion[any_region?0:lastRegion] - Duration) << endl;
			cout << NumOfSigmaTimes << " * " << sigmaRegion[any_region?0:lastRegion] << endl;
#endif

		}
		else if (type == 'A')
		{
			Point p;
			p.RegionName = nameRegion[lastRegion];
			p.Duration = lastDuration;
			file >> p.CounterID;
			file >> p.TotalCounter;

			/* Outlier could be inherited from T type */
			if (p.CounterID == "PAPI_TOT_INS" && removeOutliers && inRegion)
				Outlier = Outlier || fabs (meanRegion_tot_ins[any_region?0:lastRegion] - p.TotalCounter) > NumOfSigmaTimes*sigmaRegion_tot_ins[any_region?0:lastRegion];

#if defined(DEBUG)
			if (p.CounterID == "PAPI_TOT_INS")
			{
				cout << "ACCUMULATED " << p.TotalCounter << " REGION (" << nameRegion[lastRegion] << ")= "<< lastRegion << (Outlier?" is":" is not") << " an outlier " << endl;
				cout << "MEAN_REGION = " << meanRegion_tot_ins[any_region?0:lastRegion] << " ABS = " << fabs (meanRegion_tot_ins[any_region?0:lastRegion] - p.TotalCounter) << endl;
				cout << NumOfSigmaTimes << " * " << sigmaRegion_tot_ins[any_region?0:lastRegion] << endl;
			}
#endif

			for (unsigned i = 0 ; i < wantedCounters.size(); i++)
				if (wantedCounters[i] == p.CounterID)
					accumulatedCounterPoints.push_back (p);
		}
		else if (type == 'S')
		{
			Sample s;

			s.Phase = 0;
			file >> s.CounterID;
			file >> s.Time;
			file >> s.CounterValue;
			file >> s.DeTime;
			file >> s.DeCounterValue;
			file >> s.iteration;
			s.Region = lastRegion;

#if defined(DEBUG)
			if (inRegion)
			{
				cout << "REGION " << lastRegion << " TIME " << s.Time << " COUNTERID " << s.CounterID << " COUNTERVALUE " << s.CounterValue << endl;
			}
#endif

			if (hasMaxIteration)
			{
				if (!Outlier && inRegion && s.iteration <= MaxIteration)
					vsamples.push_back (s);
			}
			else
			{
				if (!Outlier && inRegion)
					vsamples.push_back (s);
			}
		}
	}
}

void CalculateStatsFromFile (ifstream &file, bool any_region)
{
	char type;

	for (int i = 0; i < MAX_REGIONS; i++)
	{
		meanRegion[i] = meanRegion_tot_ins[i] = 0.0f;
		countRegion[i] = 0;
		sigmaRegion[i] = sigmaRegion_tot_ins[i] = 0.0f;
	}

	/* Calculate totals and number of presence of each region */

	int Region = 0;
	while (true)
	{
		file >> type;

		if (file.eof())
			break;

		if (type == 'D')
		{
			string strRegion;
			file >> strRegion;
			TranslateRegion (strRegion);
		}
		else if (type == 'T')
		{
			string strRegion;
			unsigned long long Duration;

			file >> strRegion;
			file >> Duration;

			Region = TranslateRegion (strRegion);

			meanRegion[any_region?0:Region] += Duration;
			countRegion[any_region?0:Region] ++;
		}
		else if (type == 'A')
		{
			unsigned long long unused_ll;
			string unused_s;

			file >> unused_s;
			file >> unused_ll;

			if (unused_s == "PAPI_TOT_INS")
				meanRegion_tot_ins[any_region?0:Region] += unused_ll;

		}
		else if (type == 'S')
		{
			double unused_f;
			string unused_s;

			file >> unused_s;
			file >> unused_f;
			file >> unused_f;
			file >> unused_f;
			file >> unused_f;
			file >> unused_f;
		}
	}

	/* Now calculate the means */
	for (unsigned i = 0; i < numRegions; i++)
		if (countRegion[i] > 0)
		{
			meanRegion[i] = meanRegion[i]/countRegion[i];
			meanRegion_tot_ins[i] = meanRegion_tot_ins[i]/countRegion[i];
		}

	file.clear ();
	file.seekg (0, ios::beg);

	/* Calculate totals of SUM(x[i]-meanx) */
	while (true)
	{
		file >> type;

		if (file.eof())
			break;

		if (type == 'D')
		{
			string strRegion;
			file >> strRegion;
			TranslateRegion (strRegion);
		}
		else if (type == 'T')
		{
			string strRegion;
			unsigned long long Duration;

			file >> strRegion;
			file >> Duration;

			Region = TranslateRegion (strRegion);

			sigmaRegion[(any_region?0:Region)] +=
			 (((double)Duration) - meanRegion[any_region?0:Region]) * (((double)Duration) - meanRegion[any_region?0:Region]);
		}
		else if (type == 'A')
		{
			unsigned long long unused_ll;
			string unused_s;

			file >> unused_s;
			file >> unused_ll;

			if (unused_s == "PAPI_TOT_INS")
				sigmaRegion_tot_ins[any_region?0:Region] += 
					(((double)unused_ll) - meanRegion_tot_ins[any_region?0:Region]) * (((double)unused_ll) - meanRegion_tot_ins[any_region?0:Region]);
		}
		else if (type == 'S')
		{
			double unused_f;
			string unused_s;

			file >> unused_s;
			file >> unused_f;
			file >> unused_f;
			file >> unused_f;
			file >> unused_f;
			file >> unused_f;
		}
	}

	for (unsigned i = 0; i < numRegions; i++)
	{
		if (countRegion[i] > 1)
		{
			sigmaRegion[i] = sqrt ((sigmaRegion[i]) / (countRegion[i] - 1));
			sigmaRegion_tot_ins[i] = sqrt ((sigmaRegion_tot_ins[i]) / (countRegion[i] - 1));
		}
		else
		{
			sigmaRegion[i] = sigmaRegion_tot_ins[i] = 0.0f;
		}
	}

	file.clear ();
	file.seekg (0, ios::beg);
}

void DumpParaverLine (ofstream &f, unsigned long long type,
	unsigned long long value, unsigned long long time, unsigned long long task,
	unsigned long long thread)
{
  /* 2:14:1:14:1:69916704358:40000003:0 */
  f << "2:" << task << ":1:" << task << ":" << thread << ":" << time << ":" << type << ":" << value << endl;
}

#if 0
double runInterpolationError (int depth, double position, double divider,
	int num_in_samples, double *X_samples, double *Y_samples, double min_value,
	double max_value)
#endif
double runInterpolationError (double position, int num_in_samples,
	double *X_samples, double *Y_samples, double min_value, double max_value)
{
	/* Look for the closer position to the value 'position' within
	   X_samples */
	int close_position = 0;
	for (int i = 1; i < num_in_samples; i++)
		if (fabs(X_samples[i]-position) < fabs(X_samples[close_position]-position))
			close_position = i;

	/* Calculate value for closer value in X_samples to the 'position' value */
	double Kriger_result;
	Kriger_Point (num_in_samples, X_samples, Y_samples, X_samples[close_position],
	  &Kriger_result, min_value, max_value);

	double result = (Kriger_result - Y_samples[close_position]) * 
	                (Kriger_result - Y_samples[close_position]);

#if 0
	/* Apply recursively */
	if (depth > 0)
	{
		double new_position = position - 0.5 / divider;
		result += runInterpolationError (depth-1, new_position, 2*divider,
			num_in_samples, X_samples, Y_samples, min_value, max_value);

		new_position = position + 0.5 / divider;
		result += runInterpolationError (depth-1, new_position, 2*divider,
			num_in_samples, X_samples, Y_samples, min_value, max_value);
	}
#endif

	return result;
}

double runFullInterpolationError (int num_in_samples,
	double *X_samples, double *Y_samples, double min_value, double max_value)
{
	double result = 0.0f;
	for (int i = 0; i < num_in_samples; i++)
	{
		double Kriger_result;
		Kriger_Point (num_in_samples, X_samples, Y_samples, X_samples[i],
		  &Kriger_result, min_value, max_value);
		result += (Kriger_result - Y_samples[i]) * (Kriger_result - Y_samples[i]);
	}
	return result;
}

double newInterpolationError (int num_in_samples,
	double *X_samples, double *Y_samples, double min_value, double max_value)
{
	double result = 0.0f;

	return result;

	for (int i = 0; i < num_in_samples; i += 8)
	{
		double Kriger_result;
		Kriger_Point (num_in_samples, X_samples, Y_samples, X_samples[i],
		  &Kriger_result, min_value, max_value);
		result = MAX(fabs(Kriger_result - Y_samples[i]), result);
	}
	return result;
}

bool runInterpolation (int task, int thread, ofstream &points, ofstream &interpolation,
	ofstream &slope, ofstream &prv, vector<Sample> &vsamples, string CounterID,
	unsigned counterCode, bool anyRegion, unsigned RegionID, unsigned outcount,
	unsigned long long prvStartTime, unsigned long long prvEndTime,
	unsigned long long prvAccumCounter, double *error)
{
	bool all_zeroes = true;
	int incount = 0;

	if (outcount > 0)
	{
		vector<Sample>::iterator it = vsamples.begin();
		for (; it != vsamples.end(); it++)
		{
			if (!anyRegion && (*it).CounterID == CounterID && (*it).Region == RegionID)
				incount++;
			else if (anyRegion && (*it).CounterID == CounterID)
				incount++;
		}
	}

	if (incount > 0 && outcount > 0)
	{
		double *inpoints_x = (double*) malloc ((incount+2)*sizeof(double));
		double *inpoints_y = (double*) malloc ((incount+2)*sizeof(double));
		double *outpoints  = (double*) malloc (outcount*sizeof(double));

		inpoints_x[0] = inpoints_y[0] = 0.0f;
		inpoints_x[1] = inpoints_y[1] = 1.0f;
		vector<Sample>::iterator it = vsamples.begin();
		for (incount = 2, it = vsamples.begin(); it != vsamples.end(); it++)
			if ((!anyRegion && (*it).CounterID == CounterID && (*it).Region == RegionID) || 
			    (anyRegion && (*it).CounterID == CounterID))
			{
				inpoints_x[incount] = (*it).Time;
				inpoints_y[incount] = (*it).CounterValue;

				all_zeroes = inpoints_y[incount] == 0.0f && all_zeroes;

				if (points.is_open())
					points << "INPOINTS " << (*it).Time << " " << (*it).CounterValue << " " << (*it).iteration << endl;

				incount++;
			}

		if (!all_zeroes)
		{
			/* If the region is not filled with 0s, run the regular countoring algorithm */
			cout << "CALL_KRIGER (region=";
			if (anyRegion)
				cout << "any";
			else
				cout << RegionID << " / " << nameRegion[RegionID];
			cout << ", incount=" << incount << ", outcount=" << outcount << ", hwc=" << CounterID << ")" << endl;

			Kriger_Region (incount, inpoints_x, inpoints_y, outcount, outpoints, 0.0f, 1.0f);

			if (interpolation.is_open() && slope.is_open())
			{
				interpolation << "KRIGER " << ((double) 0 / (double) outcount) << " " << outpoints[0] << endl;
				// slope << "SLOPE 0 0" << endl; /* force to start at 0? no! */
				for (unsigned j = 1; j < outcount; j++)
				{
					double d_j = (double) j;
					double d_outcount = (double) outcount;	
					interpolation << "KRIGER " << d_j / d_outcount << " " << outpoints[j] << endl;
					slope << "SLOPE " << d_j / d_outcount << " " << (outpoints[j]-outpoints[j-1])/ (d_j/d_outcount - (d_j-1)/d_outcount) << endl; 
				}
			}
		}
		else
		{
			/* If the region are only zeroes, skip interpolating and place 0s directly.
			   This will save some badly formed countoring results because of the implied
			   added 1.0f in inpoints_y[1] */

			cout << "CALL_KRIGER (region=";
			if (anyRegion)
				cout << "any";
			else
				cout << RegionID << " / " << nameRegion[RegionID];
			cout << ", incount=" << incount << ", outcount=" << outcount << ", hwc=" << CounterID << ") -- filled with zeroes" << endl;
			if (interpolation.is_open() && slope.is_open())
			{
				for (unsigned j = 0; j < outcount; j++)
				{
					interpolation << "KRIGER 0 0" << endl;
					slope << "SLOPE 0 0" << endl;
				}
			}
		}

		if (feedTraceRegion || feedTraceTimes)
		{
#warning "Afegir els POINTS a la trasa"
			unsigned long long newCounterID = 600000000 + counterCode;
			unsigned long long deltaTime = (prvEndTime - prvStartTime) / outcount;
			DumpParaverLine (prv, newCounterID, 0, prvStartTime, task+1, thread+1);

			bool first_zero = true;
			for (unsigned j = 1; j < outcount; j++)
			{
				double deltaValue = outpoints[j]-outpoints[j-1];
				if (deltaValue > 0)
				{
					DumpParaverLine (prv, newCounterID, deltaValue*prvAccumCounter, prvStartTime+j*deltaTime, task+1, thread+1);
					first_zero = false;
				}
				else
				{
					if (!first_zero)
						DumpParaverLine (prv, newCounterID, 0, prvStartTime+j*deltaTime, task+1, thread+1);
				}
			}
		}

		if (1 || option_InterpolationErrorLevel > 1)
		{
#if 0
			unsigned N = (1 << (1+option_InterpolationErrorLevel))-1;
			cout << "Checking interpolation error (" << N << " steps): " << flush;
			*error = sqrt (runInterpolationError (option_InterpolationErrorLevel,
				0.5f, 2.0f, incount, inpoints_x, inpoints_y, 0.0f, 1.0f ));
			*error = *error / (N - 1);
			cout << fixed << setprecision(3) << *error << endl;
#endif

			cout << "Checking interpolation error (" << option_InterpolationErrorLevel << " steps): " << flush;
#if 0
			int total = 0;
			double result = 0.0f;
			double part = (double) 1.0 / (double) option_InterpolationErrorLevel;
			double current_position = part; /* Don't start at 0.0f */
			while (current_position < 1.0f)
			{
				result += runInterpolationError (current_position, incount, inpoints_x, inpoints_y, 0.0f, 1.0f);
				current_position += part;
				total++;
			}
#endif

#if 0
			double result = runFullInterpolationError (incount, inpoints_x, inpoints_y, 0.0f, 1.0f);
			*error = sqrt (result) / (incount-1);
			cout << fixed << setprecision(3) << result << " / " << *error << endl;
#endif

			double result = newInterpolationError (incount, inpoints_x, inpoints_y, 0.0f, 1.0f);
			*error = result;
			cout << fixed << setprecision(3) << result << " / " << *error << endl;
		}
		else
			*error = 0.0f;

		free (inpoints_x);
		free (inpoints_y);
		free (outpoints);
	}

	return incount > 0 && outcount > 0;
}

bool runLineFolding (int task, int thread, ofstream &points, ofstream &prv,
	vector<Sample> &vsamples, string CounterID, bool anyRegion, unsigned RegionID,
	unsigned outcount, unsigned long long prvStartTime, unsigned long long prvEndTime,
	unsigned long long prvAccumCounter)
{
	unsigned long long type = (CounterID == "LINEID")?630000001:630000000;
	bool found = false;

	UNREFERENCED(outcount);
	UNREFERENCED(prvStartTime);
	UNREFERENCED(prvEndTime);
	UNREFERENCED(prvAccumCounter);

	vector<Sample>::iterator it = vsamples.begin();
	for (; it != vsamples.end(); it++)
	{
		double time = prvStartTime + ((*it).Time * (prvEndTime - prvStartTime));

		if (!anyRegion && (*it).CounterID == CounterID && (*it).Region == RegionID)
		{
			if (points.is_open())
				points << "INPOINTS " << (*it).Time << " " << (*it).CounterValue << endl;

			DumpParaverLine (prv, type, (*it).CounterValue, (unsigned long long) time,
			  task+1, thread+1);
			found = true;
		}
		else if (anyRegion && (*it).CounterID == CounterID)
		{
			if (points.is_open())
				points << "INPOINTS " << (*it).Time << " " << (*it).CounterValue << endl;

			DumpParaverLine (prv, type, (*it).CounterValue, (unsigned long long) time,
			  task+1, thread+1);
			found = true;
		}
	}

	return found;
}

void doLineFolding (int task, int thread, string filePrefix, vector<Sample> &vsamples,
	unsigned long long startTime, unsigned long long endTime, RegionInfo &regions,
	string metric)
{
	stringstream taskstream, threadstream;
	taskstream << task;
	threadstream << thread;
	string task_str = taskstream.str();
	string thread_str = threadstream.str();

	for (list<Region*>::iterator i = regions.foundRegions.begin();
	     i != regions.foundRegions.end(); i++)
	{
		string RegionName = (*i)->RegionName;
		int regionIndex = TranslateRegion (RegionName);
		string completefilePrefix = filePrefix + "." + RegionName.substr (0, RegionName.find_first_of (":[]{}() "));

		ofstream output_points;
		if (generateGNUPLOTfiles)
		{
			output_points.open ((completefilePrefix+"."+metric+".points").c_str());
			if (!output_points.is_open())
			{
				cerr << "Cannot create " << completefilePrefix+".points" << " file " << endl;
				exit (-1);
			}
		}

		ofstream output_prv;

		if (feedTraceRegion || feedTraceTimes)
			output_prv.open (TraceToFeed.c_str(), ios_base::out|ios_base::app);

		if ((feedTraceRegion || feedTraceTimes) && !output_prv.is_open())
		{
			cerr << "Cannot append to " << TraceToFeed << " file " << endl;
			exit (-1);
		}

		bool done = runLineFolding (task, thread, output_points, output_prv,
		  vsamples, metric, !SeparateValues, regionIndex, 0, (*i)->Tstart,
		  (*i)->Tend, 0);

		if (feedTraceRegion || feedTraceTimes)
			output_prv.close();

		if (generateGNUPLOTfiles)
			output_points.close();

		if (!done)
			remove (completefilePrefix.c_str());

		if (generateGNUPLOTfiles)
		{
			GNUPLOTinfo *info = new GNUPLOTinfo;
			info->done = done;
			info->interpolated = false;
			info->title = "Task " + task_str + " Thread " + thread_str + " - " + RegionName;
			info->fileprefix = completefilePrefix;
			info->metric = metric;
			info->nameregion = (*i)->RegionName;
			info->mean_duration = 0;
			info->mean_counter = 0;
			GNUPLOT.push_back (info);
		}
	}
}

void doInterpolation (int task, int thread, string filePrefix,
	vector<Point> &vpoints, vector<Sample> &vsamples, unsigned posCounterID,
	unsigned long long startTime, unsigned long long endTime,
	RegionInfo &regions)
{
	stringstream taskstream, threadstream;
	taskstream << task;
	threadstream << thread;
	string task_str = taskstream.str();
	string thread_str = threadstream.str();

	unsigned counterCode = 0;
	string CounterID = wantedCounters[posCounterID];
	if (feedTraceRegion || feedTraceTimes)
	{
		for (unsigned i = 0; i < regions.HWCnames.size(); i++)
			if (regions.HWCnames[i] == CounterID)
			{
				counterCode = regions.HWCcodes[i];
				break;
			}
		if (counterCode == 0)
		{
			cerr << "FATAL ERROR! Cannot find counter " << CounterID << " within the PCF file " << endl;
			exit (-1);
		}
	}

	for (list<Region*>::iterator i = regions.foundRegions.begin();
	     i != regions.foundRegions.end(); i++)
	{
		string RegionName = (*i)->RegionName;
		int regionIndex = TranslateRegion (RegionName);

#if defined(DEBUG)
		cout << "Treating region called " << RegionName << " (index = " << regionIndex << ")" << endl;
#endif

		string completefilePrefix = filePrefix + "." + RegionName.substr (0, RegionName.find_first_of (":[]{}() "));

		ofstream output_points, output_kriger, output_slope;
		if (generateGNUPLOTfiles)
		{
			output_points.open ((completefilePrefix+"."+CounterID+".points").c_str());
			output_kriger.open ((completefilePrefix+"."+CounterID+".interpolation").c_str());
			output_slope.open ((completefilePrefix+"."+CounterID+".slope").c_str());
		}
		ofstream output_prv;

		if (feedTraceRegion || feedTraceTimes)
			output_prv.open (TraceToFeed.c_str(), ios_base::out|ios_base::app);

		if (generateGNUPLOTfiles)
		{
			if (!output_points.is_open())
			{
				cerr << "Cannot create " << completefilePrefix+".points" << " file " << endl;
				exit (-1);
			}
			if (!output_kriger.is_open())
			{
				cerr << "Cannot create " << completefilePrefix+".interpolation" << " file " << endl;
				exit (-1);
			}
			if (!output_slope.is_open())
			{
				cerr << "Cannot create " << completefilePrefix+".slope" << " file " << endl;
				exit (-1);
			}
		}
		if ((feedTraceRegion || feedTraceTimes) && !output_prv.is_open())
		{
			cerr << "Cannot append to " << TraceToFeed << " file " << endl;
			exit (-1);
		}

		double error;
		unsigned long long num_out_points = 10000;
		unsigned long long target_num_points;
		if (feedTraceRegion || feedTraceTimes)
			target_num_points = 2+(num_out_points*((*i)->Tend - (*i)->Tstart) / (endTime - startTime));
		else
			target_num_points = 1000;

#warning "Accumulate several equal clusters!"

		bool done = runInterpolation (task, thread, output_points, output_kriger,
		  output_slope, output_prv, vsamples, CounterID, counterCode,
		  !SeparateValues, regionIndex, target_num_points, (*i)->Tstart,
		  (*i)->Tend, (*i)->HWCvalues[posCounterID], &error);

		if (feedTraceRegion || feedTraceTimes)
			output_prv.close();

		if (generateGNUPLOTfiles)
		{
			output_slope.close();
			output_points.close();
			output_kriger.close();
		}

		if (!done)
		{
			remove ((completefilePrefix+"."+CounterID+".points").c_str());
			remove ((completefilePrefix+"."+CounterID+".interpolation").c_str());
			remove ((completefilePrefix+"."+CounterID+".slope").c_str());
		}

		if (generateGNUPLOTfiles)
		{
			unsigned tmp = 0;

			GNUPLOTinfo *info = new GNUPLOTinfo;
			info->done = done;
			info->interpolated = true;
			info->title = "Task " + task_str + " Thread " + thread_str + " - " + RegionName;
			info->fileprefix = completefilePrefix;
			info->metric = CounterID;
			info->nameregion = (*i)->RegionName;

			info->mean_counter = 0;
			info->mean_duration = 0;
			for (vector<Point>::iterator it = vpoints.begin(); it != vpoints.end(); it++)
				if ((*it).CounterID == CounterID && (*it).RegionName == RegionName)
				{
					info->mean_counter += (*it).TotalCounter;
					info->mean_duration += (*it).Duration;
					tmp++;
				}
			if (tmp > 0)
			{
				info->mean_counter = info->mean_counter / tmp;
				info->mean_duration = info->mean_duration / tmp;
			}
			info->error = error;

			if (FilterMinDuration)
			{
				if (info->mean_duration < MinDuration)
				{
					remove ((completefilePrefix+"."+CounterID+".points").c_str());
					remove ((completefilePrefix+"."+CounterID+".interpolation").c_str());
					remove ((completefilePrefix+"."+CounterID+".slope").c_str());
				}
				else
					GNUPLOT.push_back (info);
			}
			else 
				GNUPLOT.push_back (info);
		}
	}
}

void dumpAccumulatedCounterData (int task, int thread, string filePrefix,
	unsigned posCounterID, vector<Point> &vpoints, vector<Sample> &vsamples,
	RegionInfo &regions)
{
	stringstream taskstream, threadstream;
	taskstream << task;
	threadstream << thread;
	string task_str = taskstream.str();
	string thread_str = threadstream.str();
	string CounterID = wantedCounters[posCounterID];

	for (list<Region*>::iterator i = regions.foundRegions.begin();
	     i != regions.foundRegions.end(); i++)
	{
		string RegionName = (*i)->RegionName;
		unsigned regionIndex = TranslateRegion (RegionName);

#if defined(DEBUG)
		cout << "Treating region called " << RegionName << " (index = " << regionIndex << ")" << endl;
#endif

		string completefilePrefix = filePrefix + "." + RegionName.substr (0, RegionName.find_first_of (":[]{}() "));

		ofstream output_data;
		if (generateGNUPLOTfiles)
		{
			output_data.open ((completefilePrefix+"."+CounterID+".acc.points").c_str());

			if (!output_data.is_open())
			{
				cerr << "Cannot create " << completefilePrefix+".acc.points" << " file " << endl;
				exit (-1);
			}
		}

#if 1
		for (vector<Sample>:: iterator it = vsamples.begin(); it != vsamples.end(); it++)
			if ((*it).CounterID == CounterID && (*it).Region == regionIndex)
				output_data << (*it).DeTime << " " << (*it).DeCounterValue << endl;
#endif

		for (vector<Point>::iterator it = vpoints.begin(); it != vpoints.end(); it++)
			if ((*it).CounterID == CounterID && (*it).RegionName == RegionName)
				output_data << (*it).Duration << " " << (*it).TotalCounter << endl;

		if (generateGNUPLOTfiles)
			output_data.close();
	}
}

int ProcessParameters (int argc, char *argv[])
{
	if (argc < 2)
	{
		cerr << "Insufficient number of parameters" << endl
		     << "Available options are: " << endl
		     << "-remove-outliers [SIGMA]" << endl
		     << "-counter ID"<< endl
		     << "-separator-value [yes/no]" << endl
		     << "-feed-region TYPE VALUE" << endl
		     << "-feed-time TIME1 TIME2" << endl
		     << "-do-line-folding [yes/no]" << endl
		     << "-interpolate-error [level (2 by default)]" << endl
		     << "-generate-gnuplot [yes/no]" << endl
		     << "-min-duration [T in ms]" << endl
		     << "-max-iteration IT" << endl
		     << endl;
		exit (-1);
	}

	for (int i = 1; i < argc-1; i++)
	{
		if (strcmp ("-generate-gnuplot", argv[i]) == 0)
		{
			i++;
			generateGNUPLOTfiles = strcmp (argv[i], "yes") == 0;
			continue;
		}
		if (strcmp ("-min-duration", argv[i]) == 0)
		{
			unsigned tmp;
			i++;
			FilterMinDuration = true;
			tmp = atoi(argv[i]);
			if (tmp == 0)
			{
				cerr << "Invalid --min-duration value (should be > 0)" << endl;
				exit (-1);
			}
			MinDuration = ((double)tmp) * 1000000;
			continue;
		}
		if (strcmp ("-separator-value",  argv[i]) == 0)
		{
			i++;
			SeparateValues = strcmp (argv[i], "yes") == 0;
			continue;
		}
		if (strcmp ("-do-line-folding", argv[i]) == 0)
		{
			i++;
			option_doLineFolding = strcmp (argv[i], "yes") == 0;
			continue;
		}
		if (strcmp (argv[i], "-interpolate-error") == 0)
		{
			i++;
			if (atoi (argv[i]) < 0)
			{
				cerr << "Invalid -interpolate-error level value (should be >= 0)" << endl;
				exit (-1);
			}
			else
				option_InterpolationErrorLevel = atoi (argv[i]);
			continue;
		}
		if (strcmp ("-counter", argv[i]) == 0)
		{
			i++;
			if (i < argc-1)
				wantedCounters.push_back (string(argv[i]));
			continue;
		}
		if (strcmp ("-remove-outliers", argv[i]) == 0)
		{
			i++;
			removeOutliers = true;
			if (atof (argv[i]) == 0.0f)
			{
				cerr << "Invalid sigma " << argv[i] << endl;
				exit (-1);
			}
			else
				NumOfSigmaTimes = atof(argv[i]);
			continue;
		}
		if (strcmp ("-feed-region", argv[i]) == 0)
		{
			feedTraceRegion = true;
			feedTraceTimes = false;
			i++;
			feedTraceRegion_Type = atoll (argv[i]);
			i++;
			feedTraceRegion_Value = atoll (argv[i]);

			if (feedTraceRegion_Type == 0 || feedTraceRegion_Value == 0)
			{
				cerr << "Invalid -feed-region type/value pair" << endl;
				exit (-1);
			}
			continue;
		}
		if (strcmp ("-feed-time", argv[i]) == 0)
		{
			feedTraceTimes = true;
			feedTraceRegion = false;
			i++;
			feedTraceTimes_Begin = atoll (argv[i]);
			i++;
			feedTraceTimes_End = atoll (argv[i]);

			if (feedTraceTimes_Begin == 0 || feedTraceTimes_End == 0)
			{
				cerr << "Invalid -feed-time TIME1 / TIME2 pair" << endl;
				exit (-1);
			}
			continue;
		}
		if (strcmp ("-max-iteration", argv[i]) == 0)
		{
			i++;
			MaxIteration = atoi (argv[i]);
			if (MaxIteration == 0)
			{
				cerr << "Invalid -max-iteration parameter" << endl;
				exit (-1);
			}
			else
				hasMaxIteration = true;
		}
		else
			cout << "Misunderstood parameter: " << argv[i] << endl;
	}

	return argc-1;
}

void GetTaskThreadFromFile (string file, unsigned *task, unsigned *thread)
{
	string tmp;

	tmp = file.substr (file.rfind ('.')+1, string::npos);
	if (tmp.length() > 0)
	{
		if (atoi (tmp.c_str()) < 0)
		{
			cerr << "Invalid thread marker in file " << file << endl;
			exit (-1);
		}
		else
			*thread = atoi (tmp.c_str());
	}
	else
	{
		cerr << "Invalid thread marker in file " << file << endl;
		exit (-1);
	}

	tmp = file.substr (file.rfind ('.', file.rfind('.')-1)+1, file.rfind ('.') - (file.rfind ('.', file.rfind('.')-1)+1));
	if (tmp.length() > 0)
	{
		if (atoi (tmp.c_str()) < 0)
		{
			cerr << "Invalid task marker in file " << file << endl;
			exit (-1);
		}
		else
			*task = atoi (tmp.c_str());
	}
	else
	{
		cerr << "Invalid task marker in file " << file << endl;
		exit (-1);
	}
}

int main (int argc, char *argv[])
{
	UIParaverTraceConfig *pcf = NULL;
	RegionInfo regions;
	unsigned long long prv_out_start, prv_out_end;
	unsigned task, thread;
	int res = ProcessParameters (argc, argv);

	GetTaskThreadFromFile (argv[res], &task, &thread);

	ifstream InputFile (argv[res]);
	if (!InputFile.is_open())
	{
		cerr << "Unable to open " << argv[res] << endl;
		return -1;
	}

	cout << "Calculating stats" << endl;
	CalculateStatsFromFile (InputFile, !SeparateValues);

	if (feedTraceRegion || feedTraceTimes)
	{
		string cFile = argv[res];
		cFile = cFile.substr (0, cFile.rfind (".extract")) + ".control";

		ifstream controlFile (cFile.c_str());
		controlFile >> TraceToFeed;
		controlFile >> feedTraceFoldType_Value;
		controlFile.close ();

		ifstream test (TraceToFeed.c_str());
		if (!test.is_open())
		{
			cerr << "Error! Cannot open file " << TraceToFeed << endl;
			exit (-1);
		}
		test.close();	
	}

	cout << "Filling data" << endl;

	vector<Sample> vsamples;
	vector<Point> accumulatedCounterPoints;
	FillData (InputFile, !SeparateValues, vsamples, accumulatedCounterPoints);

	if (feedTraceRegion)
	{
		/* If a trace is given, search within the trace where to do the folding */
		string pcffile = TraceToFeed.substr (0, TraceToFeed.length()-3) + string ("pcf");
		pcf = new UIParaverTraceConfig (pcffile);

		SearchForRegionsWithinRegion (TraceToFeed, task, thread,
		  feedTraceFoldType_Value, feedTraceRegion_Type, feedTraceRegion_Value,
		  &prv_out_start, &prv_out_end, wantedCounters, regions, pcf);
	}
	else if (feedTraceTimes)
	{
		/* If a trace is given, search within the trace where to do the folding */
		string pcffile = TraceToFeed.substr (0, TraceToFeed.length()-3) + string ("pcf");
		pcf = new UIParaverTraceConfig (pcffile);

		SearchForRegionsWithinTime (TraceToFeed, task, thread,
		  feedTraceFoldType_Value, feedTraceTimes_Begin, feedTraceTimes_End,
		  &prv_out_start, &prv_out_end, wantedCounters, regions, pcf);
	}
	else
	{
		/* Prepare regions, as fake params for doLineFolding and doInterpolation*/
		int max = SeparateValues?numRegions:1;
		for (int i = 0; i < max; i++)
		{
			Region *r = new Region (0, 0, i);
			if (SeparateValues)
			{
				/* r->RegionName = nameRegion[i].substr (0, nameRegion[i].find_first_of (":[]{}() ")); */
				r->RegionName = nameRegion[i];
			}
			else
				r->RegionName = "all";
			for (unsigned j = 0; j < wantedCounters.size(); j++)
				r->HWCvalues.push_back (0);
			regions.foundRegions.push_back (r);
		}
	}

#if defined(DEBUG)
		cout << "# of regions: " << regions.foundRegions.size() << endl;
#endif

	for (unsigned i = 0; i < wantedCounters.size(); i++)
	{
#if defined(DEBUG)
		cout << "Working on counter " << i << " - " << wantedCounters[i] << endl;
#endif

		doInterpolation (task, thread, argv[res], accumulatedCounterPoints,
		  vsamples, i, prv_out_start, prv_out_end, regions);
		dumpAccumulatedCounterData (task, thread, argv[res], i,
		  accumulatedCounterPoints, vsamples, regions);
	}

	if (option_doLineFolding)
	{
		doLineFolding (task, thread, argv[res], vsamples, 0, 0, regions, "LINE");
		doLineFolding (task, thread, argv[res], vsamples, 0, 0, regions, "LINEID");
	}

	if (GNUPLOT.size() > 0)
	{
		createSingleGNUPLOT (argv[res], GNUPLOT);
		if (SeparateValues)
			createMultipleGNUPLOT (GNUPLOT);
		for (unsigned j = 0; j < numRegions; j++)
		{
			createMultiSlopeGNUPLOT (argv[res], nameRegion[j], GNUPLOT, wantedCounters);
//			createAccumulatedCounterGNUPLOT (argv[res], nameRegion[j], accumulatedCounterPoints, wantedCounters);
		}
	}

	return 0;
}
