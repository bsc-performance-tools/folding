#ifndef GNUPLOTGENERATOR_H_INCLUDED
#define GNUPLOTGENERATOR_H_INCLUDED

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
};

#endif /* GNUPLOTGENERATOR_H_INCLUDED */
