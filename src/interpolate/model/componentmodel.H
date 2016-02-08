/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                   Folding                                 *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/
#ifndef COMPONENTMODEL_H_INCLUDED
#define COMPONENTMODEL_H_INCLUDED

#include <string>
#include <set>

#include "componentnode.H"

class ComponentModel
{
	private:
	const string Name;
	const string Color;
	const string TitleName;
	const string PlotLocation;
	const ComponentNode *CNode;

	public:
	ComponentModel (const string &Name, const string &TitleName, 
	  const string &PlotLocation, const string &Color, ComponentNode *CN);
	~ComponentModel ();

	string getName (void) const
	  { return Name; }
	string getPlotLocation (void) const
	  { return PlotLocation; }
	const ComponentNode *getComponentNode (void) const
	  { return CNode; }
	set<string> requiredCounters (void) const;
	string getTitleName (void) const
	  { return TitleName; }
	string getColor (void) const
	  { return Color; }
	bool hasColor (void) const
	  { return Color.length() > 0; }
};

#endif /* COMPONENTMODEL_H_INCLUDED */
