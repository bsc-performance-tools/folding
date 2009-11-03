#include <UIParaverTraceConfig.h>
#include <iostream>
#include <list>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace domain;

class Information
{
	public:
	string function_name;
	string file_name;
	unsigned line;
	unsigned value;

	Information (string &function_name, string &file_name, unsigned line, unsigned value);
	~Information ();
};

Information::Information (string &function_name, string &file_name, unsigned line, unsigned value)
{
	this->function_name = function_name;
	this->file_name = file_name;
	this->line = line;
	this->value = value;
}

Information::~Information ()
{
}

int main (int argc, char *argv[])
{
	list<Information> li;

	if (argc != 3)
	{
		cerr << "Usage:" << endl << argv[0] << " pcffile eventtype" << endl;
		return -1;
	}

	UIParaverTraceConfig *pcf = new UIParaverTraceConfig (argv[1]);
	vector<unsigned> v = pcf->getEventValuesFromEventTypeKey (atoi(argv[2]));

	unsigned i = 2;
	while (i != v.size())
	{
		string ev = pcf->getEventValue(atoi(argv[2]), i);

		string func_name = ev.substr(0,ev.find(' '));
		string file_name = ev.substr(ev.find('(')+1,ev.find(':')-ev.find ('(')-1);
		unsigned line = atoi ((ev.substr(ev.find (':')+1,ev.find(')')-ev.find (':')-1)).c_str());

		list<Information>::iterator it = li.begin();
		bool inserted = false;
		while (it != li.end() && !inserted)
		{
#if defined(ABSOLUTE_FILE_LINE)
			if ((*it).file_name > file_name)
			{
				li.insert (it, *(new Information(func_name, file_name, line, i)));
				inserted = true;
			}
			else if ((*it).file_name == file_name && (*it).line > line)
			{
				li.insert (it, *(new Information(func_name, file_name, line, i)));
				inserted = true;
			}
#else
			if ((*it).function_name > func_name)
			{
				li.insert (it, *(new Information(func_name, file_name, line, i)));
				inserted = true;
			}
			else if ((*it).function_name == func_name && (*it).file_name > file_name)
			{
				li.insert (it, *(new Information(func_name, file_name, line, i)));
				inserted = true;
			}
			else if ((*it).function_name == func_name && (*it).file_name == file_name && (*it).line > line)
			{
				li.insert (it, *(new Information(func_name, file_name, line, i)));
				inserted = true;
			}
#endif
			it++;
		}
		if (!inserted)
			li.push_back (*(new Information(func_name, file_name, line, i)));

		i++;
	}

	cout << "EVENT_TYPE" << endl;
	cout << "0    " << argv[2] << "    " << pcf->getEventType(atoi(argv[2])) << endl;
	cout << "VALUES" << endl;

	cout << "0   " << pcf->getEventValue(atoi(argv[2]), 0) << endl;
	cout << "1   " << pcf->getEventValue(atoi(argv[2]), 1) << endl;
	i = 2;

	list<Information>::iterator it = li.begin();
	while (it != li.end())
	{
		cout << i << "   " << pcf->getEventValue(atoi(argv[2]), (*it).value) << endl;
		i++; it++;
	}
	cout << endl;
}

