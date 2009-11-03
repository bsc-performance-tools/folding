#ifndef PROCESS_H
#define PROCESS_H

#include "ParaverTrace.h"
#include "ParaverTraceThread.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceApplication.h"

#include "FunctionBursts.h"
#include "informationholder.H"

namespace domain
{

class Process : public ParaverTrace
{
	public:
	Process (string prvFile, bool multievents);

	list<unsigned long long> listids;

	void processMultiEvent (struct multievent_t &e);

	void DumpParaverLine (fstream &f, unsigned long long type, unsigned long long value, unsigned long long time, unsigned long long task, unsigned long long thread);

	void dumpCounters_splitIteration (unsigned task, unsigned thread);

	void DumpInformation (string fileprefix);
	void ReadInformation (string fileprefix);

	InformationHolder ih;
	~Process()
	{ };
};

} /* end of namespace domain */

#endif
