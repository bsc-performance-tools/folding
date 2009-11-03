
#include "informationholder.H"

using namespace domain;

InformationHolder::~InformationHolder()
{
	for (unsigned i = 0; i < numTasks; i++)
		delete [] TasksInfo;
}

void InformationHolder::AllocateTasks (unsigned numTasks)
{
	this->numTasks = numTasks;
	TasksInfo = new TaskInformation[this->numTasks];
}

