#ifndef GNUPLOTGENERATOR_H_INCLUDED
#define GNUPLOTGENERATOR_H_INCLUDED

#include "interpolate.H"
#include "instance-container.H"
#include "instance-group.H"
#include "variable-info.H"

using namespace std;

class gnuplotGenerator
{
	public:
	static void gnuplot_single (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string &prefix,
	  const string &counter,
	  InterpolationResults *idata,
	  const string & TimeUnit);
	static string gnuplot_slopes (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string &prefix,
	  bool per_instruction,
	  const string & TimeUnit);
	static string gnuplot_model (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string & prefix,
	  const Model *m,
	  const string & TimeUnit);
	static string gnuplot_addresses (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string & prefix,
	  const string & TimeUnit,
	  unsigned long long minaddress,
	  unsigned long long maxaddress,
	  vector<VariableInfo*> & variables);
	static string gnuplot_callers (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string & prefix,
	  const string & TimeUnit);
	static void gnuplot_groups (
	  InstanceContainer *ic,
	  const ObjectSelection *os,
	  const string & prefix,
	  StatisticType_t type);
};

#endif /* GNUPLOTGENERATOR_H_INCLUDED */
