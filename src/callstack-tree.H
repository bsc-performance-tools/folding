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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/callstackanalysis.C $
 | 
 | @last_commit: $Date: 2013-05-24 16:08:28 +0200 (dv, 24 mai 2013) $
 | @version:     $Revision: 1764 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CALLSTACKTREE_H_INCLUDED
#define CALLSTACKTREE_H_INCLUDED

#include "sample.H"

class CallstackTree
{
	private:
	CodeRefTriplet nodeTriplet;
	vector<CallstackTree*> children;
	unsigned occurrences;

	CallstackTree* findDeepestCommonCaller_r (Sample *s, unsigned depth,
	  unsigned& depth_common, CallstackTree* parent);

	public:
	CallstackTree (Sample *s, unsigned depth = 0);
	CallstackTree (void);

	void show (unsigned depth = 0);

	CodeRefTriplet getCodeRefTriplet (void) const
	  { return nodeTriplet; }
	vector<CallstackTree*> getChildren (void)
	  { return children; }

	void insert (CallstackTree *other);

	CallstackTree* findDeepestCommonCaller (Sample *s, unsigned depth, unsigned & depth_common);

	void increaseOccurrences (unsigned c = 1)
	  { occurrences += c; }
	unsigned getOccurrences (void) const
	  { return occurrences; }
};

#endif /* CALLSTACKTREE_H_INCLUDED */
