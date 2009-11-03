
#include "FunctionBursts.h"
#include <iostream>

void FunctionBursts::ResetBurst()
{
	currentburst = 0;
}

void FunctionBursts::SetBurst (unsigned long long function, list<unsigned long long> &validfunctions, unsigned long long time)
{
	list<unsigned long long>::iterator it1;
	bool found = false;

	/* Is the function on the valid list? */
	for (it1 = validfunctions.begin(); !found && it1 != validfunctions.end(); it1++)
		found = (function == *it1);
	if (!found)
		return;

	/* Go to the current burst */
	list<Function>::iterator it2 = Bursts.begin();
	for (unsigned int i = 0; i < currentburst; i++)
		it2++;

	/* Advance until we find the function in the list */
	while (it2 != Bursts.end())
	{
		if (function != (*it2).getID())
		{
			it2++;
			currentburst++;
		}
		else
			break;
	}

	/* Did we find the function ? */
	if (it2 == Bursts.end())
	{
		/* No, create an entry */
		Function f(function);
		f.setMaxTime (time);
		f.setMinTime (time);
		Bursts.push_back(f);
	}
	else
	{
		/* Yes */
		(*it2).setMaxTime (time);
		(*it2).setMinTime (time);
	}
}

void FunctionBursts::CloseIteration (unsigned long long time)
{
	bool trobat = false;

	/* Search the burst where the closing iteration time breaks it */
	list<Function>::iterator it2 = Bursts.begin();
	while (!trobat)
	{
		trobat = (*it2).getMaxTime() > time;
		if (!trobat)
			it2++;
		if (it2 == Bursts.end())
			break;
	}
	if (trobat)
		(*it2).setMaxTime (time);

	/* i amb els altres que es fa? */
}

