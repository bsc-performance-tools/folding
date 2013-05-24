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
 | @file: $HeadURL$
 | 
 | @last_commit: $Date$
 | @version:     $Revision$
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef TREENODEHOLDER_H_INCLUDED
#define TREENODEHOLDER_H_INCLUDED

#include "common.H"

//class ca_callstacksample;

#include "callstackanalysis.H"

#if HAVE_CUBE
# include <cube3/Cube.h>
#endif


class TreeNodeHolder
{
	public:
	unsigned CallerLine;
	unsigned Caller;
	pair<unsigned long long, unsigned long long> CallerLineASTBlock;
#if HAVE_CUBE
	cube::Cnode *CubeNode;
#endif

	vector<TreeNodeHolder *> children;
	unsigned Occurrences;

	TreeNodeHolder ()
	{};

	~TreeNodeHolder ()
	{};

	TreeNodeHolder *lookForCallerLine (unsigned);
	void AddPath (unsigned depth, ca_callstacksample &, UIParaverTraceConfig *pcf, cube::Cube *cubev, cube::Cnode *root, string &sourceDir);
	void AddPathAST (unsigned depth, ca_callstacksample &ca, UIParaverTraceConfig *pcf, cube::Cube *cubev, cube::Cnode *root, string &sourceDir);
};

#endif
