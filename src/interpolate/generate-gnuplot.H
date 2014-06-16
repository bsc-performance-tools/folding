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
	  const string & TimeUnit,
	  const map<unsigned,string> & hParaverIdRoutine);
	static string gnuplot_slopes (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string &prefix,
	  bool per_instruction,
	  const string & TimeUnit,
	  const map<unsigned,string> & hParaverIdRoutine);
	static string gnuplot_model (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string & prefix,
	  const Model *m,
	  const string & TimeUnit,
	  const map<unsigned,string> & hParaverIdRoutine);
	static string gnuplot_addresses (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string & prefix,
	  const string & TimeUnit,
	  vector<VariableInfo*> & variables,
	  const map<unsigned,string> & hParaverIdRoutine);
	static string gnuplot_addresses_cost (
	  InstanceGroup *ig,
	  const ObjectSelection *os,
	  const string & prefix,
	  const string & TimeUnit,
	  vector<VariableInfo*> & variables,
	  const map<unsigned,string> & hParaverIdRoutine);
	static void gnuplot_groups (
	  InstanceContainer *ic,
	  const ObjectSelection *os,
	  const string & prefix,
	  StatisticType_t type);

	static void gnuplot_routine_plot (
	  ofstream & gplot,
	  const string & fileDump,
	  InstanceGroup *ig,
	  const map<unsigned,string> & hParaverIdRoutine);
};

#endif /* GNUPLOTGENERATOR_H_INCLUDED */
