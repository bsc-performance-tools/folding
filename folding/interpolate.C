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

#include "UIParaverTraceConfig.h"
#include "kriger_wrapper.h"
#include "generate-gnuplot.H"
#include "region-analyzer.H"
#include "common.H"

#define MAX_REGIONS 1024

using namespace std;

class Sample
{
	public:
	unsigned Region;
	unsigned Phase;
	string counterID;
	float Time;
	float counterValue;
};

string TraceToFeed;
bool feedTraceRegion = false;
unsigned long long feedTraceRegion_Type;
unsigned long long feedTraceRegion_Value;
unsigned long long feedTraceFoldType_Value;

unsigned numRegions = 0;
string nameRegion[MAX_REGIONS];
int countRegion[MAX_REGIONS];
double meanRegion[MAX_REGIONS];
double sigmaRegion[MAX_REGIONS];
double NumOfSigmaTimes;

bool removeOutliers = false;
bool SeparateValues = true;
vector<string> wantedCounters;
list<GNUPLOTinfo*> GNUPLOT;

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

void FillData (ifstream &file, bool any_region, vector<Sample> &vsamples)
{
	bool inRegion = false;
	unsigned long long lastRegion = 0;
	bool Outlier = false;
	char type;

	while (true)
	{
		file >> type;
		if (file.eof())
			break;

		if (type == 'T')
		{
			string strRegion;
			unsigned long long Duration;

			file >> strRegion;
			file >> Duration;

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
		else if (type == 'S')
		{
			Sample s;

			s.Phase = 0;
			file >> s.counterID;
			file >> s.Time;
			file >> s.counterValue;
			s.Region = lastRegion;

#if defined(DEBUG)
			if (inRegion)
			{
				cout << "REGION " << lastRegion << " TIME " << s.Time << " COUNTERID " << s.counterID << " COUNTERVALUE " << s.counterValue << endl;
			}
#endif

			if (!Outlier && inRegion)
				vsamples.push_back (s);
		}
	}
}

void CalculateSigmaFromFile (ifstream &file, bool any_region)
{
	char type;

	for (int i = 0; i < MAX_REGIONS; i++)
	{
		meanRegion[i] = 0.0f;
		countRegion[i] = 0;
		sigmaRegion[i] = 0.0f;
	}

	/* Calculate totals and number of presence of each region */
	while (true)
	{
		file >> type;

		if (file.eof())
			break;

		if (type == 'T')
		{
			string strRegion;
			unsigned long long Duration;

			file >> strRegion;
			file >> Duration;

      int Region = TranslateRegion (strRegion);

			meanRegion[any_region?0:Region] += Duration;
			countRegion[any_region?0:Region] ++;
		}
		else if (type == 'S')
		{
			double unused_f;
			string unused_s;

			file >> unused_s;
			file >> unused_f;
			file >> unused_f;
		}
	}

	/* Now calculate the means */
	for (unsigned i = 0; i < numRegions; i++)
		if (countRegion[i] > 0)
			meanRegion[i] = meanRegion[i]/countRegion[i];

	file.clear ();
	file.seekg (0, ios::beg);

	/* Calculate totals of SUM(x[i]-meanx) */
	while (true)
	{
		file >> type;

		if (file.eof())
			break;

		if (type == 'T')
		{
			string strRegion;
			int Region;
			unsigned long long Duration;

			file >> strRegion;
			file >> Duration;

			Region = TranslateRegion (strRegion);

			sigmaRegion[(any_region?0:Region)] +=
			 (((double)Duration) - meanRegion[any_region?0:Region]) * (((double)Duration) - meanRegion[any_region?0:Region]);
		}
		else if (type == 'S')
		{
			double unused_f;
			string unused_s;

			file >> unused_s;
			file >> unused_f;
			file >> unused_f;
		}
	}

	for (unsigned i = 0; i < numRegions; i++)
		if (countRegion[i] > 1)
			sigmaRegion[i] = sqrt ((sigmaRegion[i]) / (countRegion[i] - 1));
		else
			sigmaRegion[i] = 0;

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

bool runInterpolation (int task, int thread, ofstream &points, ofstream &interpolation,
	ofstream &slope, ofstream &prv, vector<Sample> &vsamples, string counterID,
	unsigned counterCode, bool anyRegion, unsigned RegionID, unsigned outcount,
	unsigned long long prvStartTime, unsigned long long prvEndTime,
	unsigned long long prvAccumCounter)
{
	int incount = 0;

	if (outcount > 0)
	{
		vector<Sample>::iterator it = vsamples.begin();
		for (; it != vsamples.end(); it++)
		{
			if (!anyRegion && (*it).counterID == counterID && (*it).Region == RegionID)
				incount++;
			else if (anyRegion && (*it).counterID == counterID)
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
			if ((!anyRegion && (*it).counterID == counterID && (*it).Region == RegionID) || 
			    (anyRegion && (*it).counterID == counterID))
			{
				inpoints_x[incount] = (*it).Time;
				inpoints_y[incount] = (*it).counterValue;
				incount++;
				points << "INPOINTS " << (*it).Time << " " << (*it).counterValue << endl;
			}

		cout << "CALL_KRIGER (region=";
		if (anyRegion)
			cout << "any";
		else
			cout << RegionID << " / " << nameRegion[RegionID];
		cout << ", incount=" << incount << ", outcount=" << outcount << ", hwc=" << counterID << ")" << endl;

		Kriger_Region (incount, inpoints_x, inpoints_y, outcount, outpoints, 0.0f, 1.0f);

		interpolation << "KRIGER " << ((double) 0 / (double) outcount) << " " << outpoints[0] << endl;
		slope << "SLOPE 0 0 " << endl;
		for (unsigned j = 1; j < outcount; j++)
		{
			double d_j = (double) j;
			double d_outcount = (double) outcount;	
			interpolation << "KRIGER " << d_j / d_outcount << " " << outpoints[j] << endl;
			slope << "SLOPE " << d_j / d_outcount << " " << (outpoints[j]-outpoints[j-1])/ (d_j/d_outcount - (d_j-1)/d_outcount) << endl; 
		}

		if (feedTraceRegion)
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

		free (inpoints_x);
		free (inpoints_y);
		free (outpoints);
	}

	return incount > 0 && outcount > 0;
}

bool runLineFolding (int task, int thread, ofstream &points, ofstream &prv,
	vector<Sample> &vsamples, string counterID, bool anyRegion, unsigned RegionID,
	unsigned outcount, unsigned long long prvStartTime, unsigned long long prvEndTime,
	unsigned long long prvAccumCounter)
{
	bool found = false;

	UNREFERENCED(outcount);
	UNREFERENCED(prvStartTime);
	UNREFERENCED(prvEndTime);
	UNREFERENCED(prvAccumCounter);

	vector<Sample>::iterator it = vsamples.begin();
	for (; it != vsamples.end(); it++)
	{
		if (!anyRegion && (*it).counterID == counterID && (*it).Region == RegionID)
		{
			points << "INPOINTS " << (*it).Time << " " << (*it).counterValue << endl;
			DumpParaverLine (prv, 0 /* TYPE */, 0 /* VALUE */, 0 /* TIME */, task+1, thread+1);
			found = true;
		}
		else if (anyRegion && (*it).counterID == counterID)
		{
			points << "INPOINTS " << (*it).Time << " " << (*it).counterValue << endl;
			DumpParaverLine (prv, 0 /* TYPE */, 0 /* VALUE */, 0 /* TIME */, task+1, thread+1);
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
		string completefilePrefix = filePrefix + "." + RegionName.substr (0, RegionName.find_first_of (":[]{}() "));
		ofstream output_points ((completefilePrefix+"."+metric+".points").c_str());
		ofstream output_prv;

		if (feedTraceRegion)
			output_prv.open (TraceToFeed.c_str(), ios_base::out|ios_base::app);

		if (!output_points.is_open())
		{
			cerr << "Cannot create " << completefilePrefix+".points" << " file " << endl;
			exit (-1);
		}
		if (feedTraceRegion && !output_prv.is_open())
		{
			cerr << "Cannot append to " << TraceToFeed << " file " << endl;
		exit (-1);
		}

		bool done = runLineFolding (task, thread, output_points, output_prv,
		  vsamples, metric, !SeparateValues, (*i)->Value, 0, 0, 0, 0);

		if (feedTraceRegion)
			output_prv.close();
		output_points.close();

		if (!done)
			remove (completefilePrefix.c_str());

		GNUPLOTinfo *info = new GNUPLOTinfo;
		info->done = done;
		info->interpolated = false;
		info->title = "Task " + task_str + " Thread " + thread_str + " - " + RegionName;
		info->fileprefix = completefilePrefix;
		info->metric = metric;
		info->nameregion = (*i)->RegionName;
		GNUPLOT.push_back (info);
	}
}

void doInterpolation (int task, int thread, string filePrefix,
	vector<Sample> &vsamples, unsigned posCounterID,
	unsigned long long startTime, unsigned long long endTime,
	RegionInfo &regions)
{
	stringstream taskstream, threadstream;
	taskstream << task;
	threadstream << thread;
	string task_str = taskstream.str();
	string thread_str = threadstream.str();

	unsigned counterCode = 0;
	string counterID = wantedCounters[posCounterID];
	if (feedTraceRegion)
	{
		for (unsigned i = 0; i < regions.HWCnames.size(); i++)
			if (regions.HWCnames[i] == counterID)
			{
				counterCode = regions.HWCcodes[i];
				break;
			}
		if (counterCode == 0)
		{
			cerr << "FATAL ERROR! Cannot find counter " << counterID << " within the PCF file " << endl;
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

		ofstream output_points ((completefilePrefix+"."+counterID+".points").c_str());
		ofstream output_kriger ((completefilePrefix+"."+counterID+".interpolation").c_str());
		ofstream output_slope ((completefilePrefix+"."+counterID+".slope").c_str());
		ofstream output_prv;

		if (feedTraceRegion)
			output_prv.open (TraceToFeed.c_str(), ios_base::out|ios_base::app);

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
		if (feedTraceRegion && !output_prv.is_open())
		{
			cerr << "Cannot append to " << TraceToFeed << " file " << endl;
			exit (-1);
		}

		unsigned long long num_out_points = 1000;
		unsigned long long target_num_points;
		if (feedTraceRegion)
			target_num_points = 2+(num_out_points*((*i)->Tend - (*i)->Tstart) / (endTime - startTime));
		else
			target_num_points = num_out_points;

#warning "Accumulate several equal clusters!"

		bool done = runInterpolation (task, thread, output_points, output_kriger,
		  output_slope, output_prv, vsamples, counterID, counterCode,
		  !SeparateValues, regionIndex, target_num_points, (*i)->Tstart,
		  (*i)->Tend, (*i)->HWCvalues[posCounterID]);

		if (feedTraceRegion)
			output_prv.close();
		output_slope.close();
		output_points.close();
		output_kriger.close();

		if (!done)
		{
			remove ((completefilePrefix+"."+counterID+".points").c_str());
			remove ((completefilePrefix+"."+counterID+".interpolation").c_str());
			remove ((completefilePrefix+"."+counterID+".slope").c_str());
		}

		GNUPLOTinfo *info = new GNUPLOTinfo;
		info->done = done;
		info->interpolated = true;
		info->title = "Task " + task_str + " Thread " + thread_str + " - " + RegionName;
		info->fileprefix = completefilePrefix;
		info->metric = counterID;
		info->nameregion = (*i)->RegionName;
		GNUPLOT.push_back (info);
	}
}

int ProcessParameters (int argc, char *argv[])
{
  if (argc < 2)
  {
    cerr << "Insufficient number of parameters" << endl;
    cerr << "Available options are: " << endl
		     << "-remove-outliers [SIGMA]" << endl
         << "-counter ID"<< endl
         << "-separator-value [yes/no]" << endl
		     << "-feed-region TYPE VALUE" << endl
		     << endl;
    exit (-1);
  }

  for (int i = 1; i < argc-1; i++)
  {
    if (strcmp ("-separator-value",  argv[i]) == 0)
    {
      i++;
			SeparateValues = strcmp (argv[i], "yes") == 0;
			continue;
		}
		else if (strcmp ("-counter", argv[i]) == 0)
		{
			i++;
			if (i < argc-1)
				wantedCounters.push_back (string(argv[i]));
			continue;
		}
		else if (strcmp ("-remove-outliers", argv[i]) == 0)
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
		else if (strcmp ("-feed-region", argv[i]) == 0)
		{
			feedTraceRegion = true;
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

	if (removeOutliers)
		CalculateSigmaFromFile (InputFile, !SeparateValues);

	if (feedTraceRegion)
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

	vector<Sample> vsamples;
	FillData (InputFile, !SeparateValues, vsamples);

	if (feedTraceRegion)
	{
		string pcffile = TraceToFeed.substr (0, TraceToFeed.length()-3) + string ("pcf");
		pcf = new UIParaverTraceConfig (pcffile);

		SearchForRegionsWithinRegion (TraceToFeed, task, thread,
		  feedTraceFoldType_Value, feedTraceRegion_Type, feedTraceRegion_Value,
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
				// r->RegionName = nameRegion[i].substr (0, nameRegion[i].find_first_of (":[]{}() "));
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

		doInterpolation (task, thread, argv[res], vsamples, i, prv_out_start,
		  prv_out_end, regions);	
	}

#warning "This should be optional"
	if (1 /* should be optional */)
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
			createMultiSlopeGNUPLOT (argv[res], nameRegion[j], GNUPLOT, wantedCounters);
	}

	return 0;
}
