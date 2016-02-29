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
#include "pcf-common.H"

#include "interpolate.H"

#include "folding-reader.H"
#include "instance-container.H"
#include "prv-writer.H"
#include "prv-types.H"

#include "sample-selector-first.H"
#include "sample-selector-distance.H"
#include "sample-selector-distance-fast.H"
#include "sample-selector-default.H"

#include "instance-separator-none.H"
#include "instance-separator-auto.H"
#if defined(HAVE_CLUSTERING_SUITE)
# include "instance-separator-dbscan.H"
#endif

#include "callstack-processor.H"
#include "callstack-processor-consecutive-recursive.H"
#include "callstack-processor-consecutive-recursive.H"

#include "interpolation-kriger.H"
#include "interpolation-R-strucchange.H"
#if defined(HAVE_CUBE)
# include "callstack.H"
# include "cube-holder.H"
#endif
#include "model.H"

#include "data-object.H"

#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <iomanip>

#include <assert.h>
#include <time.h>
#include <stdlib.h>

static string TimeUnit;

static ObjectSelection *objectsSelected;

static vector<Model*> models;

static SampleSelectorDefault ssdefault; 
static SampleSelector *ss = &ssdefault;

static InterpolationKriger ik(1000, 0.0001, false);
static Interpolation *interpolation = &ik;

enum FeedType_t { FEED_NONE, FEED_TIME, FEED_FIRST_OCCURRENCE };
static FeedType_t feedTraceType = FEED_NONE;
static unsigned long long feedTraceTimes_Begin, feedTraceTimes_End;
static ObjectSelection *objectToFeed = NULL;

static InstanceSeparatorNone isnone (true);
static InstanceSeparatorAuto isauto_all (true);
static InstanceSeparatorAuto isauto_lead (false);
static InstanceSeparator *instanceseparator = &isnone;

static StatisticType_t StatisticType = STATISTIC_MEAN; 
static double NumOfSigmaTimes = 2.0f;

static set<string> wantedCounters;
static set<string> wantedRegions, wantedRegionsStartWith;

#if defined(CALLSTACK_ANALYSIS)
static bool wannaCallstackProcessing = false;
static unsigned CallstackProcessor_nconsecutivesamples = 7;
static unsigned long long CallstackProcessor_duration = 1;
static double CallstackProcessor_pct = 2.5;
typedef enum {
	CALLSTACKPROCESSOR_NONE,
	CALLSTACKPROCESSOR_CONSECUTIVE_DURATION,
	CALLSTACKPROCESSOR_CONSECUTIVE_PCT
} CallstackProcessor_type_t;
static CallstackProcessor_type_t CallstackProcessor_type = CALLSTACKPROCESSOR_CONSECUTIVE_PCT;
#endif

FoldedParaverTrace *ftrace = NULL;

static unsigned feedTraceFoldType;
static string feedTraceFoldType_Definition;

static string sourceDirectory;

static bool needAddressPCFinfo = false;

using namespace std;


void GroupFilterAndDumpStatistics (set<string> &regions,
	const vector<Instance*> &vInstances,
	map<string, InstanceContainer> &Instances,
	vector<Instance*> &feedInstances)
{
	map<string, InstanceContainer*> ptrInstances;

	cout << "Allocating instances into instance container (" << vInstances.size() << ") ... " << flush;
	for (unsigned u = 0; u < vInstances.size(); u++)
	{
		if ((u+1) % 100000 == 0)
			cout << (u+1)/1000 << "k ... " << flush;

		string Region = vInstances[u]->getRegionName();
		if (ptrInstances.count (Region) == 0)
		{
			/* If first instance with this name */
#if 0
			InstanceContainer ic (Region, instanceseparator);
			ic.add (vInstances[u]);
			Instances.insert (pair<string, InstanceContainer> (Region, ic));
#else
			InstanceContainer *ic = new InstanceContainer (Region, instanceseparator);
			ic->add(vInstances[u]);
			ptrInstances.insert (pair<string, InstanceContainer*> (Region, ic));
#endif

		}
		else
		{
			/* If not-first instance with this name */
#if 0
			InstanceContainer ic = Instances.at(Region);
			ic.add (vInstances[u]);
			Instances.at(Region) = ic;
#else
			InstanceContainer *ic = ptrInstances.at(Region);
			ic->add (vInstances[u]);
#endif
		}
	}
	cout << "Done!" << endl;

	for (const auto & i : ptrInstances)
		Instances.insert (pair<string, InstanceContainer> (i.first, *i.second));

	cout << "Detecting groups in instances ... " << flush;
	for (auto const & region : regions)
		if (Instances.count(region) > 0)
		{
			InstanceContainer ic = Instances.at(region);
			ic.splitInGroups ();
			Instances.at(region) = ic;
		}
	cout << "Done!" << endl;

	cout << fixed << setprecision (3) << endl << "Statistics for the extracted data" << endl << "-----" << endl;

	for (auto const & region : regions)
	{
		if (Instances.count(region) == 0)
			continue;

		cout << "Analysis for region named : " << region << endl;

		InstanceContainer ic = Instances.at(region);

		cout << " No. of Groups for this region : " << ic.numGroups() << endl;
		for (unsigned u = 0; u < ic.numGroups(); u++)
		{
			cout << " Analysis for " << instanceseparator->nameGroup (u)
				  << " (" << u+1 << " of " << ic.numGroups() << ")" << endl;

			InstanceGroup *ig = ic.getInstanceGroup (u);

			cout << "  No. of Instances = " << ig->numInstances() << " for a total of " << ig->numSamples() << " samples";
			if (StatisticType == STATISTIC_MEAN)
			{
				double mean = ig->mean(), stdev = ig->stdev();
				double uplimit = mean + NumOfSigmaTimes * stdev;
				double lolimit = mean - NumOfSigmaTimes * stdev;

				cout << ", mean = " << (mean/1000000.f) << "ms stdev = " 
				  << (stdev/1000000.f) << "ms" << endl;

				unsigned total = ig->numInstances();
				if (total > 0)
				{
					/* Remove while traversing */
					unsigned within = 0;
					vector<Instance*> vinstances = ig->getInstances();
					vector<Instance*>::iterator iter = vinstances.begin();
					while (iter != vinstances.end())
					{
						if ((*iter)->getDuration() >= lolimit && (*iter)->getDuration() <= uplimit)
						{
							within++;
							iter++;
						}
						else
						{
							ig->moveToExcluded (*iter);
							iter = vinstances.erase (iter);
						}
					}

					mean = ig->mean();
					stdev = ig->stdev();
					cout << "  No. of Instances within mean+/-"
					  << NumOfSigmaTimes << "*stdev = [ " << (lolimit/1000000.f) << "ms, "
					  << (uplimit / 1000000.f) << "ms ] = " << within << " ~ " << (within*100)/total
					  << "% of the population, mean = " << (mean/1000000.f) 
					  << "ms stdev = " << (stdev/1000000.f) << "ms" << endl;
				}
			}
			else if (StatisticType == STATISTIC_MEDIAN)
			{
				double median = ig->median(), mad = ig->MAD();
				double uplimit = median + NumOfSigmaTimes * mad;
				double lolimit = median - NumOfSigmaTimes * mad;

				cout << ", median = " << (median / 1000000.f) << "ms mad = " 
				  << (mad / 1000000.f) << "ms" << endl;

				unsigned total = ig->numInstances();
				if (total > 0)
				{
					/* Count & Remove excluded instances while traversing them */
					unsigned within = 0;
					vector<Instance*> vinstances = ig->getInstances();
					vector<Instance*>::iterator iter = vinstances.begin();
					while (iter != vinstances.end())
					{
						if ((*iter)->getDuration() >= lolimit &&
						    (*iter)->getDuration() <= uplimit )
						{
							within++;
							iter++;
						}
						else
						{
							ig->moveToExcluded (*iter);
							iter = vinstances.erase (iter);
						}
					}

					median = ig->median();
					mad = ig->MAD();
					cout << "  No. of Instances within median +/-" << NumOfSigmaTimes 
					  << "*mad = [ " << (lolimit / 1000000.f) << "ms, "
					  << (uplimit / 1000000.f)<< "ms ] = " << within << " ~ "
					  << (within*100)/total << "% of the population, median = "
					  << (median / 1000000.f) << "ms mad = " << (mad / 1000000.f) << "ms" << endl;
				}
			}
		}
		cout << "-----" << endl;
	}
	cout << endl;

	/* At this point, we receive feedInstances which are filtered according
	   to the user request. Now that we have excluded some instances, we
	   have to make sure that feedInstances contain only instances that are
	   not excluded */
	vector<Instance*>::iterator fiterator = feedInstances.begin();
	while (fiterator != feedInstances.end())
	{
		bool found = false;
		for (auto const & region : regions)
			if (Instances.count(region) > 0)
			{
				InstanceContainer ic = Instances.at(region);
				const vector<Instance*> vi = ic.getInstances();
				found = find(vi.begin(), vi.end(), (*fiterator)) != vi.end();
				if (found)
					break;
			}
		if (!found)
			fiterator = feedInstances.erase (fiterator);
		else
			fiterator++;
	}
}

void AppendInformationToPCF (string file, UIParaverTraceConfig *pcf,
	set<string> &wantedCounters)
{
	ofstream PCFfile;

	PCFfile.open (file.c_str(), ios_base::out|ios_base::app);
	if (!PCFfile.is_open())
	{
		cerr << "Unable to append to: " << file << endl;
		exit (-1);
	}

	vector<unsigned> vtypes = pcf->getEventTypes();
	vector<unsigned> caller;
	vector<unsigned> callerline;
	vector<unsigned> callerlineast;
	vector<unsigned> counters;
	for (unsigned u = 0; u < vtypes.size(); u++)
		if ( vtypes[u] >= EXTRAE_SAMPLE_CALLER_MIN && vtypes[u] <= EXTRAE_SAMPLE_CALLER_MAX )
			caller.push_back (vtypes[u]);
		else if ( vtypes[u] >= EXTRAE_SAMPLE_CALLERLINE_MIN && vtypes[u] <= EXTRAE_SAMPLE_CALLERLINE_MAX )
			callerline.push_back (vtypes[u]);
		else if ( vtypes[u] >= EXTRAE_SAMPLE_CALLERLINE_AST_MIN && vtypes[u] <= EXTRAE_SAMPLE_CALLERLINE_AST_MAX )
			callerlineast.push_back (vtypes[u]);

	PCFfile << endl << "EVENT_TYPE" << endl;
	PCFfile << "0 " << FOLDED_BASE << " Folded phase" << endl;

	if (caller.size() > 0)
	{
		PCFfile << endl << "EVENT_TYPE" << endl;
		for (unsigned u = 0; u < 2*caller.size(); u++)
		{
			PCFfile << "0 " << FOLDED_BASE + EXTRAE_SAMPLE_CALLER_MIN + u << " Folded sampling caller level " << u << endl;
			PCFfile << "0 " << FOLDED_BASE + EXTRAE_SAMPLE_CALLER_MIN + EXTRAE_SAMPLE_REVERSE_DELTA + u << " Folded sampling reverse caller level " << u << endl;
		}
		PCFfile << "0 " << FOLDED_CALLER << " Folded processed caller " << endl;

		PCFfile << "VALUES" << endl;
		vector<unsigned> v = pcf->getEventValues(EXTRAE_SAMPLE_CALLER_MIN);
		for (unsigned i = 0; i < v.size(); i++)
			PCFfile << i << " " << pcf->getEventValue(EXTRAE_SAMPLE_CALLER_MIN, v[i]) << endl;
		PCFfile << endl;
	}
	if (callerline.size() > 0)
	{
		PCFfile << endl << "EVENT_TYPE" << endl;
		for (unsigned u = 0; u < 2*callerline.size(); u++)
		{
			PCFfile << "0 " << FOLDED_BASE + EXTRAE_SAMPLE_CALLERLINE_MIN + u << " Folded sampling caller line level " << u << endl;
			PCFfile << "0 " << FOLDED_BASE + EXTRAE_SAMPLE_CALLERLINE_MIN + EXTRAE_SAMPLE_REVERSE_DELTA + u << " Folded sampling reverse caller line level " << u << endl;
		}
		PCFfile << "0 " << FOLDED_CALLERLINE << " Folded processed caller line" << endl;

		PCFfile << "VALUES" << endl;
		vector<unsigned> v = pcf->getEventValues(EXTRAE_SAMPLE_CALLERLINE_MIN);
		for (unsigned i = 0; i < v.size(); i++)
			PCFfile << i << " " << pcf->getEventValue(EXTRAE_SAMPLE_CALLERLINE_MIN, v[i]) << endl;
		PCFfile << endl;
	}
	if (callerlineast.size() > 0)
	{
		PCFfile << endl << "EVENT_TYPE" << endl;
		for (unsigned u = 0; u < 2*callerlineast.size(); u++)
		{
			PCFfile << "0 " << FOLDED_BASE + EXTRAE_SAMPLE_CALLERLINE_AST_MIN + u << " Folded sampling caller line AST level " << u << endl;
			PCFfile << "0 " << FOLDED_BASE + EXTRAE_SAMPLE_CALLERLINE_AST_MIN + EXTRAE_SAMPLE_REVERSE_DELTA + u << " Folded sampling reverse caller line AST level " << u << endl;
		}

		PCFfile << "VALUES" << endl;
		vector<unsigned> v = pcf->getEventValues(EXTRAE_SAMPLE_CALLERLINE_AST_MIN);
		for (unsigned i = 0; i < v.size(); i++)
			PCFfile << i << " " << pcf->getEventValue(EXTRAE_SAMPLE_CALLERLINE_AST_MIN, v[i]) << endl;
		PCFfile << endl;
	}

#if 0
	/* We want to add special events indicating the original event value
	   from the tracefile. Careful because getEventType may raise an excpetion
	   if the data was generated from a CSV. */
	try
	{
		PCFfile << endl << "EVENT_TYPE" << endl
		  << "0 " << FOLDED_TYPE << " Folded type : " << pcf->getEventType (foldedType)
		  << endl << "VALUES" << endl;
		vector<unsigned> v = pcf->getEventValues(foldedType);
		for (unsigned i = 0; i < v.size(); i++)
			PCFfile << i << " " << pcf->getEventValue(foldedType, v[i]) << endl;
		PCFfile << endl;
	} catch (...)
	{ }
#endif

	if (needAddressPCFinfo)
	{
		PCFfile << endl << "EVENT_TYPE" << endl
		  << "0 " << FOLDED_BASE+EXTRAE_SAMPLE_ADDRESS_LD << " Folded Sampled address (load)" << endl
		  << "0 " << FOLDED_BASE+EXTRAE_SAMPLE_ADDRESS_ST << " Folded Sampled address (store)" << endl;

		PCFfile << endl << "EVENT_TYPE" << endl
		  << "0 " << FOLDED_BASE + EXTRAE_SAMPLE_ADDRESS_REFERENCE_CYCLES << " Folded memory address reference cost cycles" << endl;

		vector<unsigned> v = pcf->getEventValues (EXTRAE_SAMPLE_ADDRESS_MEM_LEVEL);
		if (!v.empty())
		{
			PCFfile << endl << "EVENT_TYPE" << endl
			  << "0 " << FOLDED_BASE + EXTRAE_SAMPLE_ADDRESS_MEM_LEVEL
			  << " Folded memory hierarchy location for sampled address" << endl
			  << "VALUES" << endl;
			for (unsigned i = 0; i < v.size(); i++)
				PCFfile << i << " "
				  << pcf->getEventValue (EXTRAE_SAMPLE_ADDRESS_MEM_LEVEL, v[i])
				  << endl;
		}

		v = pcf->getEventValues (EXTRAE_SAMPLE_ADDRESS_TLB_LEVEL);
		if (!v.empty())
		{
			PCFfile << endl << "EVENT_TYPE" << endl
			  << "0 " << FOLDED_BASE + EXTRAE_SAMPLE_ADDRESS_TLB_LEVEL
			  << " Folded TLB hierarchy location for sampled address" << endl
		  	  << "VALUES" << endl;
			for (unsigned i = 0; i < v.size(); i++)
				PCFfile << i << " "
				  << pcf->getEventValue (EXTRAE_SAMPLE_ADDRESS_TLB_LEVEL, v[i])
				  << endl;
		}

	}

	PCFfile 
	  << endl << "EVENT_TYPE" << endl
	  << "0 " << FOLDED_INSTANCE_GROUP << " Folded instance group ID" << endl
	  << endl << "EVENT_TYPE" << endl
	  << "0 " << FOLDED_PHASE << " Folded phase" << endl
	  << endl;

	PCFfile << endl << "EVENT_TYPE" << endl;
	for (const auto cname : wantedCounters)
	{
		unsigned long long tmp = pcfcommon::lookForCounter (cname, pcf);
		if (tmp != 0)
			PCFfile << "0 " << FOLDED_BASE + tmp << " Folded " <<  cname << endl;
	}
	PCFfile << endl;

	PCFfile.close();
}

int ProcessParameters (int argc, char *argv[])
{
#define CHECK_ENOUGH_ARGS(N, argc, i) \
	(((argc) - 1 - (i)) > (N))

	/* Set the default time unit here, if in BSS - outside the app segfaults in
	   __static_initialization_and_destruction_0 */
	TimeUnit = common::DefaultTimeUnit;

	if (argc < 2)
	{
		cerr << "Insufficient number of parameters" << endl
		     << "Available options are: " << endl
		     << "-split-instances [no by default]" << endl
		     << "             no" << endl
		     << "             auto lead?" << endl
#if defined(HAVE_CLUSTERING_SUITE)
		     << "             dbscan minpoints epsilon lead?" << endl
#endif
		     << "-use-object PTASK.TASK.THREAD [where PTASK, TASK and THREAD = * by default" << endl
		     << "-use-median" << endl
		     << "-use-mean" << endl
		     << "-sigma-times X           [where X = 2.0 by default]" << endl
		     << "-feed-time TIME1 TIME2 PTASK.TASK.THREAD/any" << endl
		     << "-feed-first-occurrence PTASK.TASK.THREAD/any" << endl
		     << "-max-samples NUM" << endl
		     << "-max-samples-distance NUM" << endl
			 << "-model XMLfile" << endl
		     << "-region R                [where R = all by default]" << endl
		     << "-region-start-with R     [nothing by default]" << endl
		     << "-counter C               [where C = all by default]" << endl
		     << "-interpolation " << endl
			 << "          kriger STEPS NUGET PREFILTER?" << endl
		     << "          R-strucchange STEPS H" << endl
			 << " [DEFAULT kriger 1000 0.0001 no]" << endl
		     << "-source D                [location of the source code]" << endl
			 << "-time-unit CTR           [specify alternate time measurement / CTR]" << endl
#if defined(CALLSTACK_ANALYSIS)
             << "-callstack-processor duration n d [ where n = num of consecutive samples, d = duration]" << endl
             << "                     pct      n p [ where n = num of consecutive samples, p = percentage in 0..100]" << endl
             << " [DEFAULT pct 7 2.5]" << endl
             << " [DEFAULT 5 1]" << endl
#endif
		     << endl;
		exit (-1);
	}

	for (int i = 1; i < argc-1; i++)
	{
		if (strcmp ("-split-instances", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -split-instances parameter" << endl;
				exit (-1);
			}
			i++;

			if (strcasecmp (argv[i], "no") == 0)
			{
				instanceseparator = &isnone;
			}
			else if (strcasecmp (argv[i], "auto") == 0)
			{
				if (!CHECK_ENOUGH_ARGS(1, argc, i))
				{
					cerr << "Insufficient arguments for -split-instances auto parameter" << endl;
					exit (-1);
				}

				i++;
				string lead = argv[i];
				if (lead != "yes" && lead != "no")
				{
					cerr << "Invalid leading group selection value '" << lead << "'. Turning into 'no'." << endl;
					lead = "no";
				}

				if (lead == "yes")
					instanceseparator = &isauto_lead;
				else
					instanceseparator = &isauto_all;
			}
#if defined(HAVE_CLUSTERING_SUITE)
			else if (strcasecmp (argv[i], "dbscan") == 0)
			{
				if (!CHECK_ENOUGH_ARGS(3, argc, i))
				{
					cerr << "Insufficient arguments for -split-instances dbscan parameter" << endl;
					exit (-1);
				}
				unsigned minpoints;
				double eps;
				i++;
				if ((minpoints = atoi (argv[i])) == 0)
				{
					cerr << "Invalid minpoints for dbscan option (" << argv[i] << ")" << endl;
					exit (-1);
				}
				i++;
				if ((eps = atof (argv[i])) == 0)
				{
					cerr << "Invalid eps for dbscan option (" << argv[i] << ")" << endl;
					exit (-1);
				}
				i++;
				string lead = argv[i];
				if (lead != "yes" && lead != "no")
				{
					cerr << "Invalid leading group selection value '" << lead << "'. Turning into 'no'." << endl;
					lead = "no";
				}
				InstanceSeparatorDBSCAN *isdbscan = new InstanceSeparatorDBSCAN (minpoints, eps, lead != "yes");
				instanceseparator = isdbscan;
			}
#else
			else if (strcasecmp (argv[i], "dbscan") == 0)
			{
				cerr << "Sorry, instance algorithm is dbscan, but the package was not configured with its support!" << endl;
				exit (-1);
			}
#endif
			else
			{
				cerr << "Unknown parameter " << argv[i] << " for -split-instances" << endl;
				exit (-1);
			}
		}
		else if (strcmp ("-source", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -source parameter" << endl;
				exit (-1);
			}
			i++;
			sourceDirectory = argv[i];
			if (sourceDirectory[0] != '/')
			{
				char buffer[1024];
				char *ptr;
				ptr = getcwd (buffer, sizeof(buffer));
				sourceDirectory = string(ptr) + "/" + sourceDirectory;
			}
			if (common::existsDir (sourceDirectory))
				common::CleanMetricsDirectory (sourceDirectory);
			else
				cerr << "Cannot find directory " << sourceDirectory << endl;
		}
		else if (strcmp ("-use-object", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -use-object parameter" << endl;
				exit (-1);
			}
			i++;
			string o = argv[i];
			unsigned ptask, task, thread;
			bool aptask, atask, athread;
			if (!common::decomposePtaskTaskThreadWithAny (o, ptask, aptask, task, atask, thread, athread))
			{
				cerr << "Cannot translate " << o << " into Paraver object triplet application.task.thread" << endl;
				exit (-1);
			}
			if (objectsSelected != NULL)
				delete objectsSelected;

			objectsSelected = new ObjectSelection (ptask, aptask, task, atask,
			  thread, athread);
			continue;
		}
		else if (strcmp ("-use-median", argv[i]) == 0)
		{
			StatisticType = STATISTIC_MEDIAN;
		}
		else if (strcmp ("-use-mean", argv[i]) == 0)
		{
			StatisticType = STATISTIC_MEAN;
		}
		else if (strcmp ("-counter", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -counter parameter" << endl;
				exit (-1);
			}
			i++;
			if (i < argc-1)
				wantedCounters.insert (string(argv[i]));
		}
		else if (strcmp ("-region", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -region parameter" << endl;
				exit (-1);
			}
			i++;
			if (i < argc-1)
				wantedRegions.insert (string(argv[i]));
		}
		else if (strcmp ("-region-start-with", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -region-start-with parameter" << endl;
				exit (-1);
			}
			i++;
			if (i < argc-1)
				wantedRegionsStartWith.insert (string(argv[i]));
		}
		else if (strcmp ("-sigma-times", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -sigma-times parameter" << endl;
				exit (-1);
			}

			i++;
			if (atof (argv[i]) == 0.0f)
			{
				cerr << "Invalid sigma " << argv[i] << endl;
				exit (-1);
			}
			else
				NumOfSigmaTimes = atof(argv[i]);
		}
		else if (strcmp ("-max-samples", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -max-samples parameter" << endl;
				exit (-1);
			}

			i++;
			int numsamples;
			if ((numsamples = atoi (argv[i])) == 0)
			{
				cerr << "Invalid -max-samples" << argv[i] << endl;
				exit (-1);
			}

			SampleSelectorFirst *ssf = new SampleSelectorFirst (numsamples);
			ss = ssf;
		}
		else if (strcmp ("-max-samples-distance", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -max-samples-distance parameter" << endl;
				exit (-1);
			}

			i++;
			int numsamples;
			if ((numsamples = atoi (argv[i])) == 0)
			{
				cerr << "Invalid -max-samples-distance" << argv[i] << endl;
				exit (-1);
			}

			SampleSelectorDistance *ssd = new SampleSelectorDistance (numsamples);
			ss = ssd;
		}
		else if (strcmp ("-max-samples-distance-fast", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -max-samples-distance-fast parameter" << endl;
				exit (-1);
			}

			i++;
			int numsamples;
			if ((numsamples = atoi (argv[i])) == 0)
			{
				cerr << "Invalid -max-samples-distance-fast" << argv[i] << endl;
				exit (-1);
			}

			SampleSelectorDistanceFast *ssdf = new SampleSelectorDistanceFast (numsamples);
			ss = ssdf;
		}
		else if (strcmp ("-feed-time", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(3, argc, i))
			{
				cerr << "Insufficient arguments for -feed-time parameter" << endl;
				exit (-1);
			}

			feedTraceType = FEED_TIME;
			i++;
			feedTraceTimes_Begin = atoll (argv[i]);
			i++;
			feedTraceTimes_End = atoll (argv[i]);
			if (feedTraceTimes_Begin == 0 || feedTraceTimes_End == 0)
			{
				cerr << "Invalid -feed-time TIME1 / TIME2 pair" << endl;
				exit (-1);
			}

			i++;
			string o = argv[i];
			if (o != "any")
			{
				unsigned ptask, task, thread;
				if (!common::decomposePtaskTaskThread (o, ptask, task, thread))
				{
					cerr << "Cannot translate " << o << " into Paraver object triplet application.task.thread for -feed-time" << endl;
					exit (-1);
				}
				objectToFeed = new ObjectSelection (ptask, task, thread);
			}
			else
			{
				// if "any", create a filter for any ptask/task/thread
				objectToFeed = new ObjectSelection ();
			}
		}
		else if (strcmp ("-feed-first-occurrence", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -feed-first-occurrence parameter" << endl;
				exit (-1);
			}

			feedTraceType = FEED_FIRST_OCCURRENCE;

			i++;
			string o = argv[i];
			if (o != "any")
			{
				unsigned ptask, task, thread;
				if (!common::decomposePtaskTaskThread (o, ptask, task, thread))
				{
					cerr << "Cannot translate " << o << " into Paraver object triplet application.task.thread for -feed-first-occurrence" << endl;
					exit (-1);
				}
				objectToFeed = new ObjectSelection (ptask, task, thread);
			}
			else
			{
				// if "any", create a filter for any ptask/task/thread
				objectToFeed = new ObjectSelection ();
			}
		}
		else if (strcmp ("-time-unit", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -time-unit parameter" << endl;
				exit (-1);
			}

			i++;
			TimeUnit = string (argv[i]);
			continue;
		}
		else if (strcmp ("-interpolation", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -interpolation parameter" << endl;
				exit (-1);
			}

			i++;
			if (strcmp (argv[i], "kriger") == 0)
			{
				if (!CHECK_ENOUGH_ARGS(3, argc, i))
				{
					cerr << "Insufficient arguments for -interpolation kriger parameter" << endl;
					exit (-1);
				}

				unsigned NSTEPS;
				double NUGET;
				i++;
				if ((NSTEPS = atoi (argv[i])) == 0)
				{
					cerr << "Invalid num steps for KRIGER interpolation (" << argv[i] << ")" << endl;
					exit (-1);
				}
				i++;
				if ((NUGET = atof (argv[i])) == 0)
				{
					cerr << "Invalid nuget for KRIGER interpolation (" << argv[i] << ")" << endl;
					exit (-1);
				}
				i++;
				string strprefilter;
				if (strcasecmp (argv[i], "yes") == 0)
					strprefilter = "yes";

				cout << "Selected interpolation algorithm: Kriger (steps = " << NSTEPS << ", nuget = " << NUGET << ", prefilter = " << strprefilter << ")" << endl;
				InterpolationKriger *in = new InterpolationKriger (NSTEPS, NUGET, strprefilter == "yes");
				interpolation = in;
			}
			else if (strcmp (argv[i], "R-strucchange") == 0)
			{
				if (!CHECK_ENOUGH_ARGS(2, argc, i))
				{
					cerr << "Insufficient arguments for -interpolation R-strucchange parameter" << endl;
					exit (-1);
				}

#if defined(HAVE_R)
				unsigned NSTEPS;
				double H;
				i++;
				if ((NSTEPS = atoi (argv[i])) == 0)
				{
					cerr << "Invalid num steps for R-strucchange interpolation (" << argv[i] << ")" << endl;
					exit (-1);
				}
				i++;
				if ((H = atof (argv[i])) == 0)
				{
					cerr << "Invalid h value for R-strucchange interpolation (" << argv[i] << ")" << endl;
					exit (-1);
				}

				cout << "Selected interpolation algorithm: R-strucchange (steps = " << NSTEPS << ", h = " << H << ")" << endl;
				InterpolationRstrucchange *in = new InterpolationRstrucchange (NSTEPS, H);
				interpolation = in;
#else
				cout << "Cannot use R-strucchange algorithm because it was not available at configure time!" << endl;
#endif
			}
			else
			{
				cerr << "Invalid interpolation algorithm" << endl;
			}
		}
		else if (strcmp ("-callstack-processor", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(3, argc, i))
			{
				cerr << "Insufficient arguments for -callstack-processor-pct" << endl;
				exit (-1);
			}

			i++;
			string cp_mode = argv[i];

			if (cp_mode == "pct")
			{
				unsigned n;
				double p;

				i++;
				if ((n = atoi(argv[i])) == 0)
				{
					cerr << "Invalid number of samples for -callstack-processor pct (" << argv[i] << ")" << endl;
					exit (-1);
				}

				i++;
				p = atof (argv[i]);

				cout << "Callstack processor analysis: " << n
				     << " consecutive samples, percentage = " << p << " %" << endl;

#if defined(CALLSTACK_ANALYSIS)
				CallstackProcessor_type = CALLSTACKPROCESSOR_CONSECUTIVE_PCT;
				CallstackProcessor_nconsecutivesamples = n;
				CallstackProcessor_pct = p;
				wannaCallstackProcessing = true;
#else
				cerr << "Warning! Callstack processor was not enabled at configure time." << endl;
#endif
			}
			else if (cp_mode == "duration")
			{
				unsigned n,d;

				i++;
				if ((n = atoi(argv[i])) == 0)
				{
					cerr << "Invalid number of samples for -callstack-processor duration (" << argv[i] << ")" << endl;
					exit (-1);
				}

				i++;
				d = atoll (argv[i]);

				cout << "Callstack processor analysis: " << n
				     << " consecutive samples, duration = " << d << " ms" << endl;

#if defined(CALLSTACK_ANALYSIS)
				CallstackProcessor_type = CALLSTACKPROCESSOR_CONSECUTIVE_DURATION;
				CallstackProcessor_nconsecutivesamples = n;
				CallstackProcessor_duration = d;
				wannaCallstackProcessing = true;
#else
				cerr << "Warning! Callstack processor was not enabled at configure time." << endl;
#endif
			}
			else
			{
				cerr << "Invalid -callstack-processor (" << cp_mode << "). Available modes are pct / duration." << endl;
				exit (-1);
			}
		}
		else if (strcmp ("-model", argv[i]) == 0)
		{
			if (!CHECK_ENOUGH_ARGS(1, argc, i))
			{
				cerr << "Insufficient arguments for -model parameter" << endl;
				exit (-1);
			}
			i++;

			Model *m = new Model;
			m->loadXML (argv[i]);
			cout << "Loaded definition for model " << m->getName() << endl;
			models.push_back (m);
		}
		else
			cout << "Misunderstood parameter: " << argv[i] << endl;
	}

	if (wantedCounters.size() == 0 && models.size() == 0)
		wantedCounters.insert (string("all"));

	if (wantedRegions.size() == 0 && wantedRegionsStartWith.size() == 0)
		wantedRegions.insert (string("all"));

	return argc-1;
#undef CHECK_ENOUGH_ARGS
}

int main (int argc, char *argv[])
{
	set<string> presentCounters, counters;
	set<string> presentRegions, regions;
	map<string, InstanceContainer> Instances;
	vector<Instance*> vInstances;
	vector<Instance*> feedInstances;
	char CWD[1024], *cwd;

	srand(time(NULL));

	cout << "Folding (interpolate) based on branch " FOLDING_SVN_BRANCH " revision " << FOLDING_SVN_REVISION << endl;

	if ((cwd = getcwd (CWD, sizeof(CWD))) == NULL)
	{
		cerr << "Sorry... can't figure my own directory!" << endl;
		exit (-1);
	}

	if (getenv ("FOLDING_HOME") == NULL)
	{
		cerr << "You must define FOLDING_HOME to execute this application" << endl;
		return -1;
	}

	objectsSelected = new ObjectSelection;

	int res = ProcessParameters (argc, argv);

	// Read samples information	
	FoldingReader::ReadSamples (argv[res], objectsSelected, TimeUnit,
	  presentCounters, presentRegions, vInstances, objectToFeed, feedInstances);

	// Read variables info
	vector<DataObject*> variables;
	FoldingReader::ReadVariables (argv[res], variables);

	// Accumulate in wantedRegions the regions to be folded, ignoring the rest
	if (wantedRegions.find (string("all")) != wantedRegions.end())
	{
		set<string>::iterator it = presentRegions.begin();
		for (; it != presentRegions.end(); it++)
			regions.insert (*it);
	}
	else
	{
		set<string>::iterator it = presentRegions.begin();
		for (; it != presentRegions.end(); it++)
		{
			set<string>::iterator it2 = wantedRegions.begin();
			for (; it2 != wantedRegions.end(); it2++)
				if (*it == *it2)
				{
					regions.insert (*it);
					break;
				}
			it2 = wantedRegionsStartWith.begin();
			for (; it2 != wantedRegionsStartWith.end(); it2++)
				if ((*it).find (*it2) != string::npos)
				{
					regions.insert (*it);
					break;
				}
		}
	}

	// If regions are given but none found, show which are available
	if (regions.size() == 0)
	{
		cerr << "Error! No regions selected through filters. Available regions in the extracted data are: " << endl;
		set<string>::iterator it;
		for (it = presentRegions.begin(); it != presentRegions.end(); it++)
			if (it != presentRegions.begin())
				cout << ", " << *it;
			else
				cout << *it;
		cout << endl;
		return -1;
	}

	// If we have to calculate models, add their required counter
	set<string> tmp;
	vector<Model*>::iterator m = models.begin();
	bool models_given = !models.empty();
	while (m != models.end())
	{
		bool first_not_found = true;
		bool all_found = true;
		set<string> requiredCounters = (*m)->requiredCounters();
		set<string>::iterator c;
		for (c = requiredCounters.begin(); c != requiredCounters.end(); c++)
		{
			if (presentCounters.find (*c) == presentCounters.end())
			{
				if (first_not_found)
				{
					cout << endl <<  "Warning! Model " << (*m)->getName() <<
					  " will not be calculated!" << endl;
					first_not_found = false;
				}
				cout << "Warning! Counter " << *c << " requested by model " << 
				  (*m)->getName() << " cannot be find in the extracted data!" <<
				  endl;
				all_found = false;
			}
			else
				tmp.insert (*c);
		}

		// If any counter was not present, show available counters and ignore the
		// model. If all are present, add into counters set
		if (!all_found)
		{
			cout << "Available counters: ";
			set<string>::iterator it;
			for (it = presentCounters.begin(); it != presentCounters.end(); it++)
				if (it != presentCounters.begin())
					cout << ", " << *it;
				else
					cout << *it;
			cout << endl << endl;

			Model *todelete = *m;

			// Remove this model, and continue
			m = models.erase (m);

			delete todelete;
		}
		else
		{
			counters.insert (tmp.begin(), tmp.end());

			// Advance to the next model
			m++;
		}
	}

	// Accumulate in wantedCounters the counters to be folded, ignoring the rest
	if (wantedCounters.find (string("all")) == wantedCounters.end())
	{
		/* If all is not given, check for those counters that actually exist in the
		  extracted data */
		set<string>::iterator it;
		for (it = wantedCounters.begin(); it != wantedCounters.end(); it++)
			if (presentCounters.find (*it) != presentCounters.end())
				counters.insert (*it);
			else
				cout << "Warning! Counter " << *it <<
				  " cannot be find in the extracted data!" << endl << endl;
	}
	else
		counters = presentCounters;

	// If counters are given but none found, show which are available
	if (!models_given && counters.size() == 0)
	{
		cerr << "Error! No counters given, or if given they are not found. " << 
		  "Available counters in the extracted data are: " << endl;
		set<string>::iterator it;
		for (it = presentCounters.begin(); it != presentCounters.end(); it++)
			if (it != presentCounters.begin())
				cout << ", " << *it;
			else
				cout << *it;
		cout << endl;
		cout << "Aborting, nothing to do ..." << endl;
		return -1;
	}
	else if (models_given && counters.size() == 0)
	{
		// There is no need to replicate the message showing the available
		// counters
		cout << "Aborting, nothing to do ..." << endl;
		return -1;
	}

	// Filter read data and show some statistics
	GroupFilterAndDumpStatistics (regions, vInstances, Instances,
	  feedInstances);

	string cFile = argv[res];
	string cFilePrefix = cFile.substr (0, cFile.rfind (".extract"));


	string controlFile = common::basename (cFile.substr (0, cFile.rfind (".extract")) + ".control");
	string objectsFile = common::basename (cFile.substr (0, cFile.rfind (".extract")) + ".objects");
	string traceFile;

	{
		ifstream control (controlFile.c_str());
		if (!control.is_open())
		{
			cerr << "Error! Cannot open file " << controlFile << " which is needed to feed the tracefile" << endl;
			exit (-1);
		}
		else
		{
			control >> traceFile;
			control >> feedTraceFoldType;
			control >> feedTraceFoldType_Definition;
			control.close();
		}
	}

	string pcfFile = traceFile.substr (0, traceFile.rfind (".prv")) + ".pcf";
	UIParaverTraceConfig *pcf = new UIParaverTraceConfig;
	pcf->parse (pcfFile);

	// Apply the folding to each region
	set<string>::iterator it;
	bool first = true;
	for (it = regions.begin(); it != regions.end(); it++)
	{
		if (Instances.count(*it) == 0)
			continue;

		InstanceContainer ic = Instances.at(*it);

		if (first)
		{
			ic.removePreviousDataFiles (objectsSelected, cFilePrefix);
			ic.getInstanceGroup(0)->removePreviousData (objectsSelected, cFilePrefix);
			first = false;
		}

		if (interpolation->preFilter())
		{
			cout << "Prefilter Interpolation [" << interpolation->details() << 
			  "] (region = " << *it << "), #out steps = 1000" << endl;
			for (unsigned u = 0; u < ic.numGroups(); u++)
			{
				struct timespec time_start, time_end;

				cout << " Selecting samples from " << instanceseparator->nameGroup (u)
				  << " (" << u+1 << " of " << ic.numGroups() << ")" << flush;

				clock_gettime (CLOCK_REALTIME, &time_start);
				interpolation->pre_interpolate (NumOfSigmaTimes, ic.getInstanceGroup(u),
				  counters);
				clock_gettime (CLOCK_REALTIME, &time_end);
				time_t delta = time_end.tv_sec - time_start.tv_sec;
				cout << ", elapsed time " << delta / 60 << "m " << delta % 60 << "s" << endl;
			}
		}

		cout << "Interpolation [" << interpolation->details() << "] (region = " <<
		  *it << "), #out steps = " << interpolation->getSteps() << endl;
		for (unsigned u = 0; u < ic.numGroups(); u++)
		{
			struct timespec time_start, time_end;
			InstanceGroup *ig = ic.getInstanceGroup(u);

			cout << " Selecting samples from " << instanceseparator->nameGroup (u)
			  << " (" << u+1 << " of " << ic.numGroups() << ")" << flush;
			
			clock_gettime (CLOCK_REALTIME, &time_start);
			ss->Select (ig, counters);
			clock_gettime (CLOCK_REALTIME, &time_end);
			time_t delta = time_end.tv_sec - time_start.tv_sec;
			cout << ", elapsed time " << delta / 60 << "m " << delta % 60 << "s" << endl;

			interpolation->interpolate (ig, counters, TimeUnit);

#if defined(CALLSTACK_ANALYSIS)
			if (wannaCallstackProcessing)
			{
				CallstackProcessor *cp = NULL;
				if (CallstackProcessor_type == CALLSTACKPROCESSOR_CONSECUTIVE_DURATION)
					cp = new CallstackProcessor_ConsecutiveRecursive (ig,
					  CallstackProcessor_nconsecutivesamples, CallstackProcessor_duration);
				else if (CallstackProcessor_type == CALLSTACKPROCESSOR_CONSECUTIVE_PCT)
					cp = new CallstackProcessor_ConsecutiveRecursive (ig,
					  CallstackProcessor_nconsecutivesamples, CallstackProcessor_pct / 100.f);

				if (cp != NULL)
				{
					cout << " Processing callstacks for region " << *it << flush;
					clock_gettime (CLOCK_REALTIME, &time_start);
					ig->prepareCallstacks (cp);
					clock_gettime (CLOCK_REALTIME, &time_end);
					time_t delta = time_end.tv_sec - time_start.tv_sec;
					cout << ", elapsed time " << delta / 60 << "m " << delta % 60 << "s" << endl;
				}

				delete cp;
			}
#endif

			ig->dumpInterpolatedData (objectsSelected, cFilePrefix, models);
			ig->dumpData (objectsSelected, cFilePrefix, pcf);
			ig->gnuplot (objectsSelected, cFilePrefix, models, TimeUnit,
			  variables, pcf);
		}

		ic.dumpGroupData (objectsSelected, cFilePrefix, TimeUnit);
		ic.gnuplot (objectsSelected, cFilePrefix, StatisticType);
		ic.python (objectsSelected, cFilePrefix, counters);

		cout << endl;
	}

	// Generate a new PRV & CUBE files for the folded data
	if (feedTraceType != FEED_NONE)
	{
		ifstream objects (objectsFile.c_str());
		if (!objects.is_open())
		{
			cerr << "Error! Cannot open file " << objectsFile << " which is needed to feed the tracefile" << endl;
			exit (-1);
		}
		else
		{
			string objectExtracted;
			bool foundObject = false;

			while (!foundObject)
			{
				objects >> objectExtracted;
				if (objects.eof())
					break;

				unsigned ptask, task, thread;
				if (common::decomposePtaskTaskThread (objectExtracted, ptask, task, thread))
					foundObject = objectToFeed->match (ptask, task, thread);
			}
			objects.close();

			if (!foundObject)
			{
				cerr << "Error! You specified to feed an object (" << objectToFeed->toString() << ") that dit not provide any data... Aborting" << endl;
				exit (-1);
			}
		}

		string oFilePRV = traceFile.substr (0, traceFile.rfind (".prv")) + ".folded.prv";
		ftrace = new FoldedParaverTrace (oFilePRV, traceFile, true);

		ftrace->parseBody();

		ftrace->DumpStartingParaverLine ();

		UIParaverTraceConfig *pcf = NULL;
		string pcfFile = traceFile.substr (0, traceFile.rfind (".prv")) + ".pcf";
		pcf = new UIParaverTraceConfig;
		pcf->parse (pcfFile);

		bool mainid_found = false;

		 /* __libc_start_main & generic_start_main are routines seen as main in
		    BG/Q machines using IBM XLF compilers */
		unsigned mainid = pcfcommon::lookForValueString (pcf,
			  EXTRAE_SAMPLE_CALLER_MIN, "__libc_start_main",
			  mainid_found);
		if (!mainid_found)
			mainid = pcfcommon::lookForValueString (pcf,
			  EXTRAE_SAMPLE_CALLER_MIN, "generic_start_main",
			  mainid_found);
		/* Default to regular main symbol */
		if (!mainid_found)
			mainid = pcfcommon::lookForValueString (pcf,
			  EXTRAE_SAMPLE_CALLER_MIN, "main", mainid_found);
		if (!mainid_found)
			mainid = 0;

		map<string, unsigned> counterCodes;
		for (const auto & c : counters)
			counterCodes[c] = pcfcommon::lookForCounter (c, pcf);
	
		vector<Instance*> whichInstancesToFeed;
		if (feedTraceType == FEED_TIME)
		{
			for (unsigned u = 0; u < feedInstances.size(); u++)
			{
				Instance *i = feedInstances[u];
				if (i->getStartTime() >= feedTraceTimes_Begin && i->getStartTime() <= feedTraceTimes_End)
				{
					bool found;
					unsigned long long pv = pcfcommon::lookForValueString (pcf,
 						feedTraceFoldType, i->getRegionName(), found);
					if (!found)
					{
						cerr << "Can't find value for '" << i->getRegionName() <<"' in type " << feedTraceFoldType << endl;
					}
					else
						i->setPRVvalue (pv);
					whichInstancesToFeed.push_back (i);
				}
			}
		}
		else if (feedTraceType == FEED_FIRST_OCCURRENCE)
		{
			set< pair<string, unsigned> > usedRegions;
			for (unsigned u = 0; u < feedInstances.size(); u++)
			{
				Instance *i = feedInstances[u];
				pair<string, unsigned> RG = make_pair (i->getRegionName(), i->getGroup());
				if (usedRegions.find(RG) == usedRegions.end())
				{
					bool found;
					unsigned long long pv = pcfcommon::lookForValueString (pcf,
 						feedTraceFoldType, i->getRegionName(), found);
					if (!found)
					{
						cerr << "Can't find value for '" << i->getRegionName() <<"' in type " << feedTraceFoldType << endl;
					}
					else
						i->setPRVvalue (pv);
					whichInstancesToFeed.push_back (i);
					usedRegions.insert (RG);
				}
			}
		}

		/* Emit callstack into the new tracefile */
		cout << "Generating folded trace for Paraver (" << cwd << "/" << common::basename (oFilePRV.c_str()) << ")" << endl;

		for (unsigned u = 0; u < whichInstancesToFeed.size(); u++)
		{
			Instance *i = whichInstancesToFeed[u];
			if (regions.find(i->getRegionName()) != regions.end() &&
			  Instances.count(i->getRegionName()) > 0)
			{
				InstanceContainer ic = Instances.at (i->getRegionName());
				InstanceGroup *ig = ic.getInstanceGroup(i->getGroup());
				ftrace->DumpGroupInfo (i, feedTraceFoldType);
				ftrace->DumpInterpolationData (i, ig, counterCodes);
				ftrace->DumpCallersInInstance (i, ig);
				needAddressPCFinfo |= ftrace->DumpAddressesInInstance (i, ig);
				ftrace->DumpCallstackProcessed (i, ig);
				ftrace->DumpReverseCorrectedCallersInInstance (i, ig);
#if defined(DAMIEN_EXPERIMENTS)
				ig->DumpReverseCorrectedCallersInInstance (i, u == 0,
				  common::basename (traceFile.substr (0, traceFile.rfind(".prv"))),
				  pcf);
#endif
				ftrace->DumpBreakpoints (i, ig);
			}
		}

		string bfileprefix = common::basename (traceFile.substr (0, traceFile.rfind(".prv")));

		/* Copy .pcf and .row files */
		ifstream ifs_pcf ((traceFile.substr (0, traceFile.rfind(".prv"))+string(".pcf")).c_str());
		if (ifs_pcf.is_open())
		{
			ofstream ofs_pcf ((bfileprefix + string(".folded.pcf")).c_str());
			ofs_pcf << ifs_pcf.rdbuf();
			ifs_pcf.close();
			ofs_pcf.close();
		}
		// AppendInformationToPCF (bfileprefix + string (".folded.pcf"), pcf, counters);

		ifstream ifs_row ((traceFile.substr (0, traceFile.rfind(".prv"))+string(".row")).c_str());
		if (ifs_row.is_open())
		{
			ofstream ofs_row ((bfileprefix + string(".folded.row")).c_str());
			ofs_row << ifs_row.rdbuf();
			ifs_row.close();
			ofs_row.close();
		}

#if defined(HAVE_CUBE)
		/* Generate a callstack tree for the CUBE program */
		string oFileCUBE = traceFile.substr (0, traceFile.rfind (".prv")) +
		  "." + objectsSelected->toString(false, "any") + ".cube";
		cout << "Generating callstack tree for CUBE (" << cwd << "/" <<
		  common::basename (oFileCUBE.c_str()) << ")" << endl;
		
		CubeHolder ch (pcf, counters);

		ch.eraseLaunch (oFileCUBE);
		for (const auto & region : regions)
			if (Instances.count(region) > 0)
			{
				InstanceContainer ic = Instances.at(region);
				for (unsigned u = 0; u < ic.numGroups(); u++)
				{
					bool found;
					Callstack *ct = new Callstack;
					ct->generate (ic.getInstanceGroup(u), found, mainid);
				}
				ch.generateCubeTree (ic, sourceDirectory);
			}
		/* Severities in Cube 4.x should be written after defining the CUBE tree
		   according to Pavel Savionkou (Cube v4.3.1 / September 2015) */
		for (const auto & region : regions)
			if (Instances.count(region) > 0)
			{
				ch.setCubeSeverities ();
				InstanceContainer ic = Instances.at(region);
				ch.dumpFileMetrics (sourceDirectory, ic);
			}
		ch.dump (oFileCUBE);
#endif
	}

	return 0;
}
