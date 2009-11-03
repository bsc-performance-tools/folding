#ifndef TASKINFORMATION_H
#define TASKINFORMATION_H

#include "threadinformation.H"

namespace domain
{

class TaskInformation
{
	private:
	unsigned numThreads;
	ThreadInformation *ThreadsInfo;

	public:
	unsigned getNumThreads (void)
	{ return numThreads; };

	ThreadInformation* getThreadsInformation (void)
	{ return ThreadsInfo; };

	void AllocateThreads (unsigned numThreads);
	~TaskInformation();
};

} /* end of namespace domain */

#endif
