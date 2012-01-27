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
#include "region-analyzer-in-frame-region.H"
#include "region-analyzer-in-time-region.H"
#include "region-analyzer-first-occurrency.H"
#include "common.H"
#include "point.H"

#define TOT_INS "PM_INST_CMPL"

#define MAX(a,b) ((a)>(b))?(a):(b)
#define MIN(a,b) ((a)<(b))?(a):(b)

#define MAX_REGIONS 1024

#define FOLDED_BASE 600000000

#define QUALITY_DISTANCE 0.01

using namespace std;

class CallstackSample
{
	public:
	float Time;
	unsigned long long Type;
	unsigned long long Value;
	unsigned Region;
};

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

vector<CallstackSample> vcallstacksamples;

bool FilterMinDuration = false;
double MinDuration;

unsigned MaxIteration = 0;
bool hasMaxIteration = false;

unsigned MaxSamples = 0;

string TraceToFeed;
bool feedTraceRegion = false;
bool feedTraceTimes = false;
bool feedFirstOccurrence = false;
unsigned long long feedTraceRegion_Type;
unsigned long long feedTraceRegion_Value;
unsigned long long feedTraceFoldType_Value;
unsigned long long feedTraceTimes_Begin, feedTraceTimes_End;
unsigned long long feedSyntheticEventsRate = 1000;

unsigned numRegions = 0;
string nameRegion[MAX_REGIONS];
int countRegion[MAX_REGIONS];
double meanRegion[MAX_REGIONS], meanRegion_tot_ins[MAX_REGIONS];
double sigmaRegion[MAX_REGIONS], sigmaRegion_tot_ins[MAX_REGIONS];
double NumOfSigmaTimes;

bool option_doLineFolding = true;
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
			if (p.CounterID == TOT_INS && removeOutliers && inRegion)
				Outlier = Outlier || fabs (meanRegion_tot_ins[any_region?0:lastRegion] - p.TotalCounter) > NumOfSigmaTimes*sigmaRegion_tot_ins[any_region?0:lastRegion];

#if defined(DEBUG)
			if (p.CounterID == TOT_INS)
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
		else if (type == 'C')
		{
			CallstackSample cs;

			file >> cs.Time;
			file >> cs.Type;
			file >> cs.Value;
			cs.Region = lastRegion;

			vcallstacksamples.push_back (cs);
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

			if (unused_s == TOT_INS)
				meanRegion_tot_ins[any_region?0:Region] += unused_ll;

		}
		else if (type == 'S')
		{
			unsigned long long unused_ll;
			double unused_f;
			string unused_s;

			file >> unused_s;
			file >> unused_f;
			file >> unused_f;
			file >> unused_ll;
			file >> unused_ll;
			file >> unused_ll;
		}
		else if (type == 'C')
		{
			double unused_f;

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

			if (unused_s == TOT_INS)
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

static double Calculate_Quality0 (int incount, double *inpoints_x, double *inpoints_y, int outcount, double *outpoints, double QD)
{
	int values_within_QD = 0;
	for (int i = 0; i < incount; i++)
	{
		int pt_x = (int) (inpoints_x[i]*outcount); /* index position X of sample i into 0..outcount-1 */
		if (((inpoints_y[i] >= outpoints[pt_x] - QD)) && (inpoints_y[i] <= (outpoints[pt_x] + QD)))
			values_within_QD++;
	}
	return (double) values_within_QD / (double) incount;
}

static double Calculate_Quality1 (int incount, double *inpoints_x, double *inpoints_y, int outcount, double *outpoints)
{
	double total_distances = 0.0f;
	for (int i = 0; i < incount; i++)	
	{
		double min_distance = 1.0f;
		for (int j = 0; j < outcount; j++)
		{
			double X_distance = inpoints_x[i]-((double) j/(double) outcount);
			double Y_distance = inpoints_y[i]-outpoints[j];
			double diagonal = sqrt (X_distance*X_distance + Y_distance*Y_distance);

			min_distance = MIN(diagonal, min_distance);
		}
		total_distances += min_distance;
	}
	return (double) total_distances / (double) incount;
}

void DumpParaverLines (ofstream &f, vector<unsigned long long> &type,
	vector<unsigned long long > &value, unsigned long long time, unsigned long long task,
	unsigned long long thread)
{
  /* 2:14:1:14:1:69916704358:40000003:0 */

  f << "2:" << task << ":1:" << task << ":" << thread << ":" << time;
	for (unsigned i = 0; i < type.size(); i++)
		f << ":" << type[i] << ":" << value[i];
	f << endl;
}

void DumpParaverLine (ofstream &f, unsigned long long type,
	unsigned long long value, unsigned long long time, unsigned long long task,
	unsigned long long thread)
{
  /* 2:14:1:14:1:69916704358:40000003:0 */
  f << "2:" << task << ":1:" << task << ":" << thread << ":" << time << ":" << type << ":" << value << endl;
}

bool runInterpolation (ofstream &points, ofstream &interpolation,
	ofstream &slope, double slope_factor, vector<Sample> &vsamples,
	string CounterID, bool anyRegion, unsigned RegionID,
	unsigned outcount, int &num_inpoints, double *outpoints,
	vector<double> &qualities)
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

	for (unsigned j = 0; j < outcount; j++)
		outpoints[j] = 0.0f;

	if (incount >= 0 && outcount > 0)
	{
		double *inpoints_x = (double*) malloc ((incount+2)*sizeof(double));
		double *inpoints_y = (double*) malloc ((incount+2)*sizeof(double));

		inpoints_x[0] = inpoints_y[0] = 0.0f;
		inpoints_x[1] = inpoints_y[1] = 1.0f;
		vector<Sample>::iterator it = vsamples.begin();
		for (incount = 2, it = vsamples.begin(); it != vsamples.end(); it++)
			if ((!anyRegion && (*it).CounterID == CounterID && (*it).Region == RegionID) || 
			    (anyRegion && (*it).CounterID == CounterID))
			{

#if 0
/* be careful with these exclusions */
				if (((*it).Time != 1.0 && (*it).CounterValue == 1.0) || 
            ((*it).Time <= 0.2 && (*it).CounterValue > 0.8))
					continue;
#endif

				inpoints_x[incount] = (*it).Time;
				inpoints_y[incount] = (*it).CounterValue;

				all_zeroes = inpoints_y[incount] == 0.0f && all_zeroes;

				if (points.is_open())
					points << CounterID << " " << (*it).Time << " " << (*it).CounterValue << " " << (*it).iteration << endl;

				incount++;
			}

		if (MaxSamples != 0 && incount > MaxSamples)
		{
			cout << "Attention! Number of samples limited to " << MaxSamples << " (incount was " << incount << ")" << endl;
			incount = MaxSamples;
		}

		if (!all_zeroes)
		{
			/* If the region is not filled with 0s, run the regular countoring algorithm */
			cout << "Running interpolation (region=";
			if (anyRegion)
				cout << "any";
			else
				cout << RegionID << " / " << nameRegion[RegionID];
			cout << ", incount=" << incount << ", outcount=" << outcount << ", hwc=" << CounterID << ")" << endl;

			Kriger_Region (incount, inpoints_x, inpoints_y, outcount, outpoints, 0.0f, 1.0f);

			cout << "Calculating Q0 w/ QD = " << fixed << setprecision(2) << QUALITY_DISTANCE << flush;
			double q0 = Calculate_Quality0 (incount, inpoints_x, inpoints_y, outcount, outpoints, QUALITY_DISTANCE);
			cout << ", Q0 = " << fixed << setprecision(2) << q0 << endl;

			cout << "Calculating Q1" << flush;
			double q1 = Calculate_Quality1 (incount, inpoints_x, inpoints_y, outcount, outpoints);
			cout << ", Q1 = " << fixed << setprecision(2) << q1 << endl;

			qualities.push_back (q0);
			qualities.push_back (q1);

			/* Correct negative points present in the interpolation */
			for (unsigned j = 0; j < outcount; j++)
				if (outpoints[j] < 0)
					outpoints[j] = 0;

			if (interpolation.is_open() && slope.is_open())
			{
				interpolation << CounterID << " " << ((double) 0 / (double) outcount) << " " << outpoints[0] << endl;
				double d_last = outpoints[0];
				for (unsigned j = 1; j < outcount; j++)
				{
					double d_j = (double) j;
					double d_outcount = (double) outcount;	
					interpolation << CounterID << " " << d_j / d_outcount << " " << outpoints[j] << endl;

					if (d_last < outpoints[j])
					{
						slope << CounterID << " " << d_j / d_outcount << " " <<
							slope_factor * (outpoints[j]-d_last) / (d_j/d_outcount - (d_j-1)/d_outcount)
							<< endl; 
						d_last = outpoints[j];
					}
					else
						slope << CounterID << " " << d_j / d_outcount << " 0" << endl;
				}
			}
		}
		else
		{
			/* If the region are only zeroes, skip interpolating and place 0s directly.
			   This will save some badly formed countoring results because of the implied
			   added 1.0f in inpoints_y[1] */

			cout << "Running interpolation (region=";
			if (anyRegion)
				cout << "any";
			else
				cout << RegionID << " / " << nameRegion[RegionID];
			cout << ", incount=" << incount << ", outcount=" << outcount << ", hwc=" << CounterID << ") -- filled with zeroes" << endl;
			if (interpolation.is_open() && slope.is_open())
			{
				interpolation << CounterID << " 0 0" << endl;
				for (unsigned j = 1; j < outcount; j++)
				{
					double d_j = (double) j;
					double d_outcount = (double) outcount;	
					interpolation << CounterID << " " << d_j / d_outcount << " 0" << endl;
					slope << CounterID << " " <<  d_j / d_outcount << " 0" << endl;
				}
			}

			qualities.push_back (0.0f); /* Q0 */
			qualities.push_back (0.0f); /* Q1 */
		}

		free (inpoints_x);
		free (inpoints_y);
	}

	num_inpoints = incount;

	return incount > 0 && outcount > 0;
}

void WriteResultsIntoTrace (int task, int thread, ofstream &prv,
	Region *r, vector<unsigned long long> &HWCcodes,
	vector<unsigned long long> &HWCtotals,
	unsigned outcount, bool callstacksamples, double *_outpoints)
{
	vector<unsigned long long> types;
	vector<unsigned long long> values;
	unsigned long long prvStartTime = r->Tstart;
	unsigned long long prvEndTime = r->Tend;
	unsigned long long deltaTime = (prvEndTime - prvStartTime) / outcount;

	for (unsigned c = 0; c < HWCcodes.size(); c++)
		values.push_back (0);
	DumpParaverLines (prv, HWCcodes, values, prvStartTime, task, thread);
	values.clear();

	bool first_zero = true;
	for (unsigned j = 1; j < outcount; j++)
	{
		for (unsigned c = 0; c < HWCcodes.size(); c++)
		{
			/* last value of outpoints[] may no be strictly 1 */
			unsigned long long fixedAccumCounter = HWCtotals[c]/_outpoints[((c+1)*outcount)-1];

			double deltaValue = _outpoints[c*outcount+j]-_outpoints[c*outcount+j-1];
			if (deltaValue > 0)
			{
				types.push_back (HWCcodes[c]);
				values.push_back (deltaValue*fixedAccumCounter);
				first_zero = false;
			}
			else
			{
				if (!first_zero)
				{
					types.push_back (HWCcodes[c]);
					values.push_back (0);
				}
			}
		}

		if (types.size() > 0)
			DumpParaverLines (prv, types, values, prvStartTime+j*deltaTime, task, thread);

		types.clear();
		values.clear();
	}

	if (callstacksamples)
	{
		unsigned long long last_time = 0;
		unsigned long long time;
		unsigned i = 0;

		deltaTime = prvEndTime - prvStartTime;

		while (i < vcallstacksamples.size())
		{
			if (vcallstacksamples[i].Region == TranslateRegion(r->RegionName))
			{
				time = prvStartTime+(float) (((float)deltaTime)*vcallstacksamples[i].Time);
				if (time == last_time)
				{
					types.push_back (vcallstacksamples[i].Type+FOLDED_BASE);
					values.push_back (vcallstacksamples[i].Value);
				}
				else
				{
					if (types.size() > 0)
						DumpParaverLines (prv, types, values, last_time, task, thread);

					types.clear();
					values.clear();

					types.push_back (vcallstacksamples[i].Type+FOLDED_BASE);
					values.push_back (vcallstacksamples[i].Value);

					last_time = time;
				}
			}
			i++;
		}

		if (types.size() > 0)
			DumpParaverLines (prv, types, values, last_time, task, thread);
	}
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
			  task, thread);
			found = true;
		}
		else if (anyRegion && (*it).CounterID == CounterID)
		{
			if (points.is_open())
				points << "INPOINTS " << (*it).Time << " " << (*it).CounterValue << endl;

			DumpParaverLine (prv, type, (*it).CounterValue, (unsigned long long) time,
			  task, thread);
			found = true;
		}
	}

	return found;
}

void doLineFolding (int task, int thread, string filePrefix, vector<Sample> &vsamples,
	RegionInfo &regions, string metric)
{
	stringstream taskstream, threadstream;
	taskstream << task;
	threadstream << thread;
	string task_str = taskstream.str();
	string thread_str = threadstream.str();

	for (list<Region*>::iterator i = regions.foundRegions.begin();
	     i != regions.foundRegions.end(); i++)
	{
		/* HSG hack */
#if 0
		if (i != regions.foundRegions.begin())
			continue;
#endif

		string RegionName = (*i)->RegionName;
		int regionIndex = TranslateRegion (RegionName);
		//string completefilePrefix = filePrefix + "." + RegionName.substr (0, RegionName.find_first_of (":[]{}() "));
		string completefilePrefix = filePrefix + "." + common::removeSpaces(RegionName);

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

		if (feedTraceRegion || feedTraceTimes || feedFirstOccurrence)
			output_prv.open (TraceToFeed.c_str(), ios_base::out|ios_base::app);

		if ((feedTraceRegion || feedTraceTimes || feedFirstOccurrence) && !output_prv.is_open())
		{
			cerr << "Cannot append to " << TraceToFeed << " file " << endl;
			exit (-1);
		}

		bool done = runLineFolding (task, thread, output_points, output_prv,
		  vsamples, metric, !SeparateValues, regionIndex, 0, (*i)->Tstart,
		  (*i)->Tend, 0);

		if (feedTraceRegion || feedTraceTimes || feedFirstOccurrence)
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
	vector<Point> &vpoints, vector<Sample> &vsamples,
	RegionInfo &regions)
{
	vector<unsigned long long> HWCcodes, HWCtotals;
	static bool first_run = true;

	stringstream taskstream, threadstream;
	taskstream << task;
	threadstream << thread;
	string task_str = taskstream.str();
	string thread_str = threadstream.str();

	for (list<Region*>::iterator i = regions.foundRegions.begin();
	     i != regions.foundRegions.end(); i++)
	{
		/* Avoid excluded phases, always odd phases? */
		if ((*i)->Phase % 2 == 1)
			continue;

		string RegionName = (*i)->RegionName;
		int regionIndex = TranslateRegion (RegionName);

		string completefilePrefix = filePrefix + "." + RegionName.substr (0, RegionName.find_first_of (":[]{}() "));

		ofstream output_points, output_kriger, output_slope;
		if (generateGNUPLOTfiles)
		{
			output_points.open ((completefilePrefix+".points").c_str());
			if (!output_points.is_open())
			{
				cerr << "Error! Cannot create " << completefilePrefix+".points! Dying..." << endl;
				exit (-1);
			}
			output_kriger.open ((completefilePrefix+".interpolation").c_str());
			if (!output_points.is_open())
			{
				cerr << "Error! Cannot create " << completefilePrefix+".interpolation! Dying..." << endl;
				exit (-1);
			}
			output_slope.open ((completefilePrefix+".slope").c_str());
			if (!output_points.is_open())
			{
				cerr << "Error! Cannot create " << completefilePrefix+".slope! Dying..." << endl;
				exit (-1);
			}

			output_points.precision(10); output_points << fixed;
			output_kriger.precision(10); output_kriger << fixed;
			output_slope.precision(10); output_slope << fixed;
		}

		unsigned long long target_num_points;
		if (feedTraceRegion || feedTraceTimes || feedFirstOccurrence)
		{
			/* SER menas Synthetic Events Rate */
			unsigned long long SER_per_ns = 1000000000 / feedSyntheticEventsRate;

			/* Add two (initial and end) */
			target_num_points = 2 + ((*i)->Tend-(*i)->Tstart)/SER_per_ns;
		}
		else
			target_num_points = feedSyntheticEventsRate;

		double *outpoints = (double*) malloc (sizeof(double)*target_num_points*wantedCounters.size());
		if (outpoints == NULL)
		{
			cerr << "Cannot allocate memory for outpoints! Dying..." << endl;
			exit (-1);
		}

		bool data_dumped = false;
		for (unsigned CID = 0; CID < wantedCounters.size(); CID++)
		{
			bool found = false;
			string CounterID = wantedCounters[CID];
			if (feedTraceRegion || feedTraceTimes || feedFirstOccurrence)
			{
				for (unsigned idx = 0; idx < regions.HWCnames.size(); idx++)
					if (regions.HWCnames[idx] == CounterID)
					{
						HWCcodes.push_back (regions.HWCcodes[idx] + FOLDED_BASE);
						found = true;
						break;
					}
				if (!found && i != regions.foundRegions.begin())
				{
					cerr << "ERROR! Cannot find counter " << CounterID << " within the PCF file " << endl;
				}
			}

			double slope_factor = 0; /* computed as mean(number of hwc events) / mean (time) */
			double this_mean_counter = 0;
			double this_mean_duration = 0;
			unsigned count_counter = 0, count_duration = 0;

			for (vector<Point>::iterator it = vpoints.begin(); it != vpoints.end(); it++)
				if ((*it).CounterID == CounterID && (*it).RegionName == RegionName)
					{
						if ((*it).TotalCounter > 0)
						{
							this_mean_counter += (*it).TotalCounter;
							count_counter++;
						}

						if ((*it).Duration > 0)
						{
							this_mean_duration += (*it).Duration;
							count_duration++;
						}
					}

			if (count_counter > 0)
				this_mean_counter = this_mean_counter / count_counter;

			HWCtotals.push_back (this_mean_counter);

			if (count_duration > 0)
				this_mean_duration = this_mean_duration / count_duration;

			if (this_mean_duration > 0)
				slope_factor = this_mean_counter / (this_mean_duration / 1000);

			if (FilterMinDuration)
			{
				if (this_mean_duration < MinDuration)
					continue;
			}

			/* We're about to write some values for this region */
			data_dumped = true;

			int num_in_points;

			vector<double> qualities;
			bool done = runInterpolation (output_points, output_kriger, output_slope,
				slope_factor, vsamples, CounterID, !SeparateValues, regionIndex,
				target_num_points, num_in_points, &outpoints[target_num_points*CID],
			  qualities);

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
				info->inpoints = num_in_points;
				info->qualities = qualities;
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

				if (FilterMinDuration)
				{
					if (info->mean_duration >= MinDuration)
						GNUPLOT.push_back (info);
				}
				else 
					GNUPLOT.push_back (info);
			}
		}

		if (generateGNUPLOTfiles)
		{
			output_slope.close();
			output_points.close();
			output_kriger.close();
		}

		if (feedTraceRegion || feedTraceTimes || feedFirstOccurrence)
		{
			ofstream output_prv;
	
			output_prv.open (TraceToFeed.c_str(), ios_base::out|ios_base::app);
			if (!output_prv.is_open())
			{
				cerr << "Cannot append to " << TraceToFeed << " file " << endl;
				exit (-1);
			}

			WriteResultsIntoTrace (task, thread, output_prv, *i, HWCcodes,
				HWCtotals, target_num_points, first_run, outpoints);

			output_prv.close();
		}

		if (!data_dumped)
		{
			remove ((completefilePrefix+".points").c_str());
			remove ((completefilePrefix+".slope").c_str());
			remove ((completefilePrefix+".interpolation").c_str());
		}

		HWCtotals.clear();
		HWCcodes.clear();

		free (outpoints);
	}

	first_run = false;
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
		output_data.open ((completefilePrefix+"."+CounterID+".acc.points").c_str());

		if (!output_data.is_open())
		{
			cerr << "Cannot create " << completefilePrefix+".acc.points" << " file " << endl;
			exit (-1);
		}

#if 1
		for (vector<Sample>:: iterator it = vsamples.begin(); it != vsamples.end(); it++)
			if ((*it).CounterID == CounterID && (*it).Region == regionIndex)
				output_data << (*it).DeTime << " " << (*it).DeCounterValue << endl;
#endif

		for (vector<Point>::iterator it = vpoints.begin(); it != vpoints.end(); it++)
			if ((*it).CounterID == CounterID && (*it).RegionName == RegionName)
				output_data << (*it).Duration << " " << (*it).TotalCounter << endl;

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
		     << "-feed-first-occurrence" << endl
		     << "-do-line-folding [yes/no]" << endl
		     << "-generate-gnuplot [yes/no]" << endl
		     << "-min-duration [T in ms]" << endl
		     << "-max-iteration IT" << endl
		     << "-max-samples NUM" << endl
		     << "-synthetic-events-rate NUM (rate, num events per second) [1000 if not given]" << endl
		     << endl;
		exit (-1);
	}

	for (int i = 1; i < argc-1; i++)
	{
		if (strcmp ("-synthetic-events-rate", argv[i]) == 0)
		{
			i++;
			feedSyntheticEventsRate = atoll (argv[i]);
			if (feedSyntheticEventsRate > 1000000000)
			{
				cerr << "Invalid -synthetic-events-rate (should be > 0 and < 1000000000)" << endl;
				exit (-1);
			}
			continue;
		}
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
				cerr << "Invalid -min-duration value (should be > 0)" << endl;
				exit (-1);
			}
			MinDuration = ((double)tmp) * 1000000;
			continue;
		}
		if (strcmp ("-max-samples", argv[i]) == 0)
		{
			i++;
			MaxSamples = atoi(argv[i]);
			if (MaxSamples == 0)
			{
				cerr << "Invalid -max-samples value (should be >0)" << endl;
				exit (-1);
			}
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
			feedFirstOccurrence = false;
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
			feedFirstOccurrence = false;
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
		if (strcmp ("-feed-first-occurrence", argv[i]) == 0)
		{
			feedTraceRegion = false;
			feedTraceTimes = false;
			feedFirstOccurrence = true;

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

void AppendInformationToPCF (string file, UIParaverTraceConfig *pcf,
	vector<string> &wantedCounters)
{
	bool any_found;
	ofstream PCFfile;

	PCFfile.open (file.c_str(), ios_base::out|ios_base::app);
	if (!PCFfile.is_open())
	{
		cerr << "Unable to append to: " << PCFfile << endl;
		exit (-1);
	}

	any_found = false;
	for (unsigned i = 30000000; i < 30000099; i++)
		if (pcf->getEventType(i) != "")
			any_found = true;
	if (any_found)
	{
		PCFfile << endl << "EVENT_TYPE" << endl;
		for (unsigned i = 30000000; i < 30000099; i++)
			if (pcf->getEventType(i) != "")
				PCFfile << "0 " << FOLDED_BASE + i << " Folded sampling caller level " << i - 30000000 << endl;
		PCFfile << "VALUES" << endl;
		PCFfile << "0 " << pcf->getEventValue(30000000, 0) << endl;
		PCFfile << "1 " << pcf->getEventValue(30000000, 1) << endl;

		vector<unsigned> v = pcf->getEventValuesFromEventTypeKey (30000000);
		for (unsigned i = 2; i < v.size(); i++)
			PCFfile << i << " " << pcf->getEventValue(30000000, i) << endl;
		PCFfile << endl;
	}

	any_found = false;
	for (unsigned i = 30000100; i < 30000199; i++)
		if (pcf->getEventType(i) != "")
			any_found = true;
	if (any_found)
	{
		PCFfile << endl <<  "EVENT_TYPE" << endl;
		for (unsigned i = 30000100; i < 30000199; i++)
			if (pcf->getEventType(i) != "")
				PCFfile << "0 " << FOLDED_BASE + i << " Folded sampling caller line level " << i - 30000100 << endl;
		PCFfile << "VALUES" << endl;
		PCFfile << "0 " << pcf->getEventValue(30000100, 0) << endl;
		PCFfile << "1 " << pcf->getEventValue(30000100, 1) << endl;

		vector<unsigned> v = pcf->getEventValuesFromEventTypeKey (30000100);
		for (unsigned i = 2; i < v.size(); i++)
			PCFfile << i << " " << pcf->getEventValue(30000100, i) << endl;

		PCFfile << endl;
	}

	PCFfile << endl << "EVENT_TYPE" << endl;
	for (unsigned i = 0 ; i < wantedCounters.size(); i++)
	{
		unsigned long long tmp = common::lookForCounter (wantedCounters[i], pcf);
		if (tmp != 0)
			PCFfile << "0 " << FOLDED_BASE + tmp << " Folded " << wantedCounters[i] << endl;
	}
	PCFfile << endl;
		

	PCFfile.close();
}

int main (int argc, char *argv[])
{
	vector<unsigned long long> phasetypes;
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

	if (feedTraceRegion || feedTraceTimes || feedFirstOccurrence)
	{
		string cFile = argv[res];
		cFile = cFile.substr (0, cFile.rfind (".extract")) + ".control";

		ifstream controlFile (cFile.c_str());
		if (!controlFile.is_open())
		{
			cerr << "Error! Cannot open file " << cFile << endl;
			exit (-1);
		}
		controlFile >> TraceToFeed;
		controlFile >> feedTraceFoldType_Value;
		{
			int numPhaseTypes;
			controlFile >> numPhaseTypes;
			for (int i = 0; i < numPhaseTypes; i++)
			{
				unsigned long long phasetype;
				controlFile >> phasetype;
				phasetypes.push_back (phasetype);
			}
		}

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

		AppendInformationToPCF (TraceToFeed.substr (0, TraceToFeed.length()-3) + string ("pcf"),
			pcf, wantedCounters);
	}
	else if (feedTraceTimes)
	{
		/* If a trace is given, search within the trace where to do the folding */
		string pcffile = TraceToFeed.substr (0, TraceToFeed.length()-3) + string ("pcf");
		pcf = new UIParaverTraceConfig (pcffile);

		SearchForRegionsWithinTime (TraceToFeed, task, thread,
		  feedTraceFoldType_Value, feedTraceTimes_Begin, feedTraceTimes_End,
		  &prv_out_start, &prv_out_end, wantedCounters, regions, pcf);

		AppendInformationToPCF (TraceToFeed.substr (0, TraceToFeed.length()-3) + string ("pcf"),
			pcf, wantedCounters);
	}
	else if (feedFirstOccurrence)
	{
		/* If a trace is given, search within the trace where to do the folding */
		string pcffile = TraceToFeed.substr (0, TraceToFeed.length()-3) + string ("pcf");
		pcf = new UIParaverTraceConfig (pcffile);

		SearchForRegionsFirstOccurrence (TraceToFeed, task, thread,
		  feedTraceFoldType_Value, wantedCounters, regions, pcf,
			phasetypes);

		AppendInformationToPCF (TraceToFeed.substr (0, TraceToFeed.length()-3) + string ("pcf"),
			pcf, wantedCounters);
	}
	else
	{
		/* Prepare regions, as fake params for doLineFolding and doInterpolation*/
		int max = SeparateValues?numRegions:1;
		for (int i = 0; i < max; i++)
		{
			Region *r = new Region (0, 0, i, 0);
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

	doInterpolation (task, thread, argv[res], accumulatedCounterPoints,
	  vsamples, regions);

	dumpAccumulatedCounterData (task, thread, argv[res], 0, accumulatedCounterPoints, vsamples, regions);

	if (option_doLineFolding)
	{
		doLineFolding (task, thread, argv[res], vsamples, regions, "LINE");
		doLineFolding (task, thread, argv[res], vsamples, regions, "LINEID");
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
