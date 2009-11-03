
#include "taskinformation.H"

using namespace domain;

TaskInformation::~TaskInformation()
{
	for (unsigned i = 0; i < numThreads; i++)
		delete [] ThreadsInfo;
}

void TaskInformation::AllocateThreads (unsigned numThreads)
{
	this->numThreads = numThreads;
	ThreadsInfo = new ThreadInformation[this->numThreads];
}

