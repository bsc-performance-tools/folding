#include <UIParaverTraceConfig.h>
#include <iostream>
#include <list>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace libparaver;

class Information
{
	public:
	string file_name;
	unsigned line;
	unsigned value;

	Information (string &file_name, unsigned line, unsigned value);
	~Information ();
};

Information::Information (string &file_name, unsigned line, unsigned value)
{
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

/*
		string func_name = ev.substr(0,ev.find(' '));
		string file_name = ev.substr(ev.find('(')+1,ev.find(':')-ev.find ('(')-1);
		unsigned line = atoi ((ev.substr(ev.find (':')+1,ev.find(')')-ev.find (':')-1)).c_str());
*/

		unsigned line = atoi ((ev.substr (0, ev.find(' '))).c_str());
		string file_name = ev.substr(ev.find('(')+1,ev.find(')')-ev.find ('(')-1);

		list<Information>::iterator it = li.begin();
		bool inserted = false;
		while (it != li.end() && !inserted)
		{
#if 0
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
#endif

			if ((*it).file_name > file_name)
			{
				li.insert (it, *(new Information(file_name, line, i)));
				inserted = true;
			}
			else if ((*it).file_name == file_name && (*it).line > line)
			{
				li.insert (it, *(new Information(file_name, line, i)));
				inserted = true;
			}
			
			it++;
		}
		if (!inserted)
			li.push_back (*(new Information(file_name, line, i)));

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

