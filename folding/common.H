#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <string>
#include "UIParaverTraceConfig.h"

#define UNREFERENCED(a) ((a) = (a))

using namespace std;
using namespace libparaver;

class common
{
	public:
	static string removeSpaces (string &in);
	static unsigned lookForCounter (string &name, UIParaverTraceConfig *pcf);
};

#endif
