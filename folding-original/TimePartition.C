
#include <iostream>

#include "TimePartition.h"

TimePartition::TimePartition (void)
{
	v_times.push_back (0);
	v_usable.push_back (false);
#if 0
	v_data.push_back (new vector<pair<unsigned long long, unsigned long long> >);
#endif
}

TimePartition::~TimePartition (void)
{
}

void TimePartition::ExtendPartition (unsigned long long newtime, bool usable)
{
	v_times.push_back (newtime);
	v_usable.push_back (usable);
#if 0
	v_data.push_back (new vector<pair<unsigned long long, unsigned long long> >);
#endif
}

int TimePartition::LookTimeInPartition (unsigned long long time)
{
	for (unsigned i = 1; i < v_times.size(); i++)
		if (v_times[i] > time)
			return v_usable[i-1]?i-1:-1;

	return -1;
}

#if 0
void TimePartition::AddData (unsigned long long time, unsigned long long data)
{
	int partition = LookTimeInPartition (time);
	if (partition != -1)
	{
		if (partition < v_data.size ())
		{
			pair<unsigned long long, unsigned long long> p = pair<unsigned long long, unsigned long long>(time, data);
			v_data[partition]->push_back (p);
		}
		else
			cerr << "Unable to find partition for time " << time << endl;
	}
}

void TimePartition::DumpData (void)
{
	for (unsigned i = 0; i < v_data.size(); i++)
	{
		cout << "PARTITION " << i << " Usable? " << v_usable[i] << " Time Limit: " << v_times[i] << endl;
		cout << "SAMPLES" << endl;
		for (unsigned j = 0; j < v_data[i]->size(); j++)
			cout << " { " << (*v_data[i])[j].first << "," << (*v_data[i])[j].second << " } " << endl;
		cout << "END PARTITION " << i << endl;
	}
}
#endif
