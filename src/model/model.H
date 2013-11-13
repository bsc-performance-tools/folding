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
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/instance-group.H $
 | 
 | @last_commit: $Date: 2013-10-29 13:39:46 +0100 (Tue, 29 Oct 2013) $
 | @version:     $Revision: 2259 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <set>
#include <string>

#include "componentmodel.H"
#include "componentnode_constant.H"
#include "componentnode_data.H"
#include "componentnode_derived.H"
#include "componentnode.H"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

using namespace std;

class Model
{
	private:
	string Name;
	string Y1AxisName;
	string Y2AxisName;
	bool Y1Stacked;
	string TitleName;

	vector< ComponentModel* > components;

	ComponentNode * loadXML_component_componentnode (xmlDocPtr xmldoc,
	  xmlNodePtr current_tag, unsigned depth = 0);
	ComponentModel * loadXML_component (xmlDocPtr xmldoc, xmlNodePtr tag);

	public:
	Model ();
	~Model ();

	bool loadXML (char *f);
	set<string> requiredCounters (void) const;

	string getName(void) const
	  { return Name; }
	string getY1AxisName (void) const
	  { return Y1AxisName; }
	string getY2AxisName (void) const
	  { return Y2AxisName; }
	bool isY1Stacked (void) const
	  { return Y1Stacked; }
	vector< ComponentModel *> getComponents (void) const
	  { return components; }
	string getTitleName(void) const
	  { return TitleName; }
};

#endif /* MODEL_H_INCLUDED */

