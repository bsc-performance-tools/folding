#include "folding-writer.H"
#include <fstream>

using namespace std;

int main (int argc, char *argv[])
{
	string nameRegion = "FunctionA";
	unsigned long long startRegion = 1000;
	unsigned long long durationRegion = 4500;

	/* NOTE:: Counters are given in deltas from their previous read, not as absolute values */
	map<string, unsigned long long> c1;
	c1["PAPI_TOT_INS"] = 1000;
	c1["PAPI_TOT_CYC"] = 2000;
	map<unsigned, CodeRefTriplet> crt1;
	Sample *s1 = new Sample (2000, 2000-startRegion, c1, crt1);

	map<string, unsigned long long> c2;
	c2["PAPI_TOT_INS"] = 1000;
	c2["PAPI_TOT_CYC"] = 2000;
	map<unsigned, CodeRefTriplet> crt2;
	Sample *s2 = new Sample (3000, 3000-startRegion, c2, crt2);

	map<string, unsigned long long> c3;
	c3["PAPI_TOT_INS"] = 1000;
	c3["PAPI_TOT_CYC"] = 2000;
	map<unsigned, CodeRefTriplet> crt3;
	Sample *s3 = new Sample (4000, 4000-startRegion, c3, crt3);

	/* Last sample typically coincides with end of region -- see durationRegion,
	   Folding::Write won't emit in a S entry */
	map<string, unsigned long long> c4;
	c4["PAPI_TOT_INS"] = 500;
	c4["PAPI_TOT_CYC"] = 1000;
	map<unsigned, CodeRefTriplet> crt4;
	Sample *s4 = new Sample (4500, 4500-startRegion, c4, crt4);

	vector<Sample*> vs;
	vs.push_back (s1);
	vs.push_back (s2);
	vs.push_back (s3);
	vs.push_back (s4);

	ofstream f("output.extract");
	if (f.is_open())
	{
		FoldingWriter::Write (f, nameRegion, 1, 1, 1, startRegion, durationRegion, vs);
		f.close();
	}

	return 0;
}
