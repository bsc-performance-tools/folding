#ifndef FUNCTIONBURSTS_H_INCLUDED
#define FUNCTIONBURSTS_H_INCLUDED

#include <iostream>
#include <list>
#include <limits.h>

using namespace std;

#define MAX(a,b) \
	((a)>(b)?(a):(b))

#define MIN(a,b) \
	((a)<(b)?(a):(b))

class Function
{
	unsigned long long functionid;
	unsigned long long mintime;
	unsigned long long maxtime;

	public:
	Function (int id)
	{ functionid = id; maxtime = 0; mintime = ULLONG_MAX; };

	void setMaxTime (unsigned long long time)
	{ maxtime = MAX(maxtime,time); };

	void forceMaxTime (unsigned long long time)
	{ maxtime = time; };

	void setMinTime (unsigned long long time)
	{ mintime = MIN(mintime,time); };

	void forceMinTime (unsigned long long time)
	{ mintime = time; };

	unsigned long long getID ()
	{ return functionid; };

	unsigned long long getMaxTime ()
	{ return maxtime; };

	unsigned long long getMinTime ()
	{ return mintime; };

	bool operator < (Function other)
	{ return getMinTime() < other.getMinTime(); };

	void dump ()
	{ cout << "ID " << getID() << " MIN " << getMinTime() << " MAX " << getMaxTime() << endl; }
};

class FunctionBursts 
{
	private:
	unsigned long long currentburst;
	list<Function> Bursts;

	public:
	list<Function> & getFunctionBursts ()
	{ return Bursts; };

	void ResetBurst();
	void CloseIteration (unsigned long long time);
	void SetBurst (unsigned long long function, list<unsigned long long> &validfunctions, unsigned long long time);
	FunctionBursts()
	{ currentburst = 0; };
};

#endif
