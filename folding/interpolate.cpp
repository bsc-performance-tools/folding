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
#include "kriger_wrapper.h"

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

int numRegions = 0;
string nameRegion[MAX_REGIONS];
int countRegion[MAX_REGIONS];
double meanRegion[MAX_REGIONS];
double sigmaRegion[MAX_REGIONS];
double NumOfSigmaTimes;

bool joinGNUplots = false;
bool removeOutliers = false;
bool SeparateValues = true;
vector<string> wantedCounters;
list<string> GNUPLOTinfo_points;

int TranslateRegion (string &RegionName)
{
	/* Look for region in the translation table */
	for (int i = 0; i < numRegions; i++)
		if (nameRegion[i] == RegionName)
			return i;

	/* If it can be find, add it */
	int result = numRegions;
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

	while (!file.eof())
	{
		file >> type;

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

#ifdef DEBUG
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

#ifdef DEBUG
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
	while (!file.eof())
	{
		file >> type;
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
	for (int i = 0; i < numRegions; i++)
		if (countRegion[i] > 0)
			meanRegion[i] = meanRegion[i]/countRegion[i];

	file.clear ();
	file.seekg (0, ios::beg);

	/* Calculate totals of SUM(x[i]-meanx) */
	while (!file.eof())
	{
		file >> type;
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

	for (int i = 0; i < numRegions; i++)
		if (countRegion[i] > 1)
			sigmaRegion[i] = sqrt ((sigmaRegion[i]) / (countRegion[i] - 1));
		else
			sigmaRegion[i] = 0;

	file.clear ();
	file.seekg (0, ios::beg);
}

void runInterpolation (ofstream &points, ofstream &interpolation,
	ofstream &slope, vector<Sample> &vsamples, string counterID,
	bool anyRegion, unsigned Region)
{
	int incount = 0, outcount = 1000;

	vector<Sample>::iterator it = vsamples.begin();
	for (; it != vsamples.end(); it++)
		if (!anyRegion && (*it).counterID == counterID && (*it).Region == Region)
			incount++;
		else if (anyRegion && (*it).counterID == counterID)
			incount++;

	if (incount > 0)
	{
		double *inpoints_x = (double*) malloc ((incount+2)*sizeof(double));
		double *inpoints_y = (double*) malloc ((incount+2)*sizeof(double));
		double *outpoints  = (double*) malloc (outcount*sizeof(double));

		inpoints_x[0] = inpoints_y[0] = 0.0f;
		inpoints_x[1] = inpoints_y[1] = 1.0f;
		for (incount = 2, it = vsamples.begin(); it != vsamples.end(); it++)
			if ((!anyRegion && (*it).counterID == counterID && (*it).Region == Region) || 
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
			cout << Region << " / " << nameRegion[Region];
		cout << " , incount=" << incount << ", outcount=" << outcount << ", hwc=" << counterID << ")" << endl;
		Kriger_Wrapper (incount, inpoints_x, inpoints_y, outcount, outpoints, 0.0f, 1.0f);

		interpolation << "KRIGER " << ((double) 0 / (double) outcount) << " " << outpoints[0] << endl;
		slope << "SLOPE 0 0 " << endl;
		for (int j = 1; j < outcount; j++)
		{
			double d_j = (double) j;
			double d_outcount = (double) outcount;	
			interpolation << "KRIGER " << d_j / d_outcount << " " << outpoints[j] << endl;
			slope << "SLOPE " << d_j / d_outcount << " " << (outpoints[j]-outpoints[j-1])/ (d_j/d_outcount - (d_j-1)/d_outcount) << endl; 
		}

		free (inpoints_x);
		free (inpoints_y);
		free (outpoints);
	}
}

void doInterpolation (string filePrefix, vector<Sample> &vsamples,
	string &counterID)
{
#ifdef DEBUG
	cout << "doInterpolation (..) with numRegions = " << numRegions << endl;
#endif

	if (SeparateValues)
	{
		for (int i = 0; i < numRegions; i++)
		{
			string choppedNameRegion = nameRegion[i].substr (0, nameRegion[i].find(":"));

/*
			stringstream regionnumber;
			regionnumber << i;
			string completefilePrefix = filePrefix + ".region." + regionnumber.str();
*/

			string completefilePrefix = filePrefix + "." + choppedNameRegion;

			ofstream output_points ((completefilePrefix+"."+counterID+".points").c_str());
			ofstream output_kriger ((completefilePrefix+"."+counterID+".interpolation").c_str());
			ofstream output_slope ((completefilePrefix+"."+counterID+".slope").c_str());

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

			GNUPLOTinfo_points.push_back (completefilePrefix);

			runInterpolation (output_points, output_kriger, output_slope, vsamples,
				counterID, false, i);

			output_slope.close();
			output_points.close();
			output_kriger.close();
		}
	}
	else
	{
		string completefilePrefix = filePrefix + ".region.all";

		ofstream output_points ((completefilePrefix+"."+counterID+".points").c_str());
		ofstream output_kriger ((completefilePrefix+"."+counterID+".interpolation").c_str());
		ofstream output_slope ((completefilePrefix+"."+counterID+".slope").c_str());

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

		GNUPLOTinfo_points.push_back (completefilePrefix);

		runInterpolation (output_points, output_kriger, output_slope, vsamples,
			counterID, true, 0);

		output_slope.close();
		output_points.close();
		output_kriger.close();
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
         << "-separator-value [yes/no]" << endl;
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
		else if (strcmp ("-joinplots", argv[i]) == 0)
		{
			joinGNUplots = true;
			continue;
		}
  }

  return argc-1;
}

void createMultipleGNUPLOT (string file, string &counterID)
{
	list<string>::iterator it = GNUPLOTinfo_points.begin();
	for (int i = 0; i < numRegions; i++, it++)
	{
		string choppedNameRegion = nameRegion[i].substr (0, nameRegion[i].find(":"));
		string GNUPLOTfile = file+"."+choppedNameRegion+"."+counterID+".gnuplot";

		ofstream gnuplot_out (GNUPLOTfile.c_str());
		if (!gnuplot_out.is_open())
		{
			cerr << "Cannot create " << GNUPLOTfile << " file " << endl;
			exit (-1);
		}

		gnuplot_out
		  << "set term x11 persist" << endl
			<< "set xrange [0:1]" << endl
		  << "set yrange [0:1]" << endl
		  << "set x2range [0:1]" << endl
		  << "set y2range [0:2]" << endl
		  << "set ytics nomirror" << endl
		  << "set xtics nomirror" << endl
		  << "set y2tics" << endl
		  << "set x2tics" << endl
		  << "set key bottom right" << endl
			<< "set title '" << nameRegion[i] << "' ;" << endl
			<< "set ylabel '" << counterID << "';" << endl
			<< "set y2label 'Slope of " << counterID << "';" << endl
			<< "set xlabel 'Normalized time';" << endl
			<< "plot '" << (*it) << "." << counterID << ".points' using 2:3 title 'Samples',"
			<<      "'" << (*it) << "." << counterID << ".interpolation' using 2:3 title 'Curve fitting' w lines lw 2,"
		 	<<      "'" << (*it) << "." << counterID << ".slope' using 2:3 title 'Curve fitting slope' axes x2y2 w lines lw 2;"
			<< endl;

		gnuplot_out.close();
	}
}

void createSingleGNUPLOT (string file, string &counterID)
{
	string GNUPLOTfile = file+".gnuplot."+counterID;

	ofstream gnuplot_out (GNUPLOTfile.c_str());
	if (!gnuplot_out.is_open())
	{
		cerr << "Cannot create " << GNUPLOTfile << " file " << endl;
		exit (-1);
	}

	gnuplot_out
	  << "set term x11 persist" << endl
		<< "set xrange [0:1]" << endl
	  << "set yrange [0:1]" << endl
	  << "set x2range [0:1]" << endl
	  << "set y2range [0:2]" << endl
	  << "set ytics nomirror" << endl
	  << "set xtics nomirror" << endl
	  << "set y2tics" << endl
	  << "set x2tics" << endl
	  << "set key bottom right" << endl;

	list<string>::iterator it = GNUPLOTinfo_points.begin();
	for (int i = 0; i < numRegions; i++, it++)
		gnuplot_out
			<< "set title '" << nameRegion[i] << "' ;" << endl
			<< "set ylabel '" << counterID << "';" << endl
			<< "set xlabel 'Normalized time';" << endl
			<< "plot '" << (*it) << "." << counterID << ".points' using 2:3 title 'Samples',"
			<<      "'" << (*it) << "." << counterID << ".interpolation' using 2:3 title 'Curve fitting' w lines lw 2,"
		 	<<      "'" << (*it) << "." << counterID << ".slope' using 2:3 title 'Curve fitting slope' axes x2y2 w lines lw 2;"
			<< endl;

	gnuplot_out.close();
}

int main (int argc, char *argv[])
{
	int res = ProcessParameters (argc, argv);

	ifstream InputFile (argv[res]);
	if (!InputFile.is_open())
	{
		cerr << "Unable to open " << argv[res] << endl;
		return -1;
	}

	if (removeOutliers)
		CalculateSigmaFromFile (InputFile, !SeparateValues);

	vector<Sample> vsamples;
	FillData (InputFile, !SeparateValues, vsamples);

	for (vector<string>::iterator it = wantedCounters.begin(); it != wantedCounters.end(); it++)
		doInterpolation (argv[res], vsamples, *it);	

	if (GNUPLOTinfo_points.size() > 0)
		for (vector<string>::iterator it = wantedCounters.begin(); it != wantedCounters.end(); it++)
			if (joinGNUplots)
				createSingleGNUPLOT (argv[res], *it);
			else
				createMultipleGNUPLOT (argv[res], *it);

	return 0;
}
