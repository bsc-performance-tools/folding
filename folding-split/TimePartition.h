
#include <vector>

using namespace std;

#if 0
typedef vector<pair<unsigned long long, unsigned long long> > * pvector;
#endif

class TimePartition
{
	private:
	vector<bool> v_usable;
	vector<unsigned long long> v_times;
#if 0
	vector<pvector> v_data;
#endif


	public:
	int LookTimeInPartition (unsigned long long time);
	void ExtendPartition (unsigned long long newtime, bool usable = true);
#if 0
	void AddData (unsigned long long time, unsigned long long data);
	void DumpData (void);
#endif

	TimePartition (void);
	~TimePartition (void);
};

