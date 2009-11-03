#ifndef INFORMATIONHOLDER_H
#define INFORMATIONHOLDER_H

#include "taskinformation.H"

namespace domain
{

class InformationHolder
{
	private:
	unsigned numTasks;
	TaskInformation *TasksInfo;
	
	public:
	unsigned getNumTasks (void)
	{ return numTasks; };

	TaskInformation* getTasksInformation (void)
	{ return TasksInfo; };

	void AllocateTasks (unsigned numTasks);
	~InformationHolder();
};

}
#endif
