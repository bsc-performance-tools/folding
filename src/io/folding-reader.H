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
#ifndef FOLDING_READER_H_INCLUDED
#define FOLDING_READER_H_INCLUDED

#include <string>
#include <set>
#include "object-selection.H"
#include "instance.H"
#include "data-object.H"

using namespace std;

class FoldingReader
{
	public:
	static void ReadSamples (const string & filenameextract,
	  const ObjectSelection *os, const string & TimeUnit,
	  set<string> &allcounters, set<string> &allregions,
	  vector<Instance*> &Instances, ObjectSelection *osfeed, 
	  vector<Instance*> &feedInstances);
	static void ReadVariables (const string & filenameextract,
	  vector<DataObject*> &dataObjects);
};

#endif /* FOLDING_READER_H_INCLUDED */

