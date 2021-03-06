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
#ifndef INSTANCE_SEPARATOR_DBSCAN_H_INCLUDED
#define INSTANCE_SEPARATOR_DBSCAN_H_INCLUDED

#include "instance-separator.H"

/* This is a subclass from InstanceSeparator. It uses DBSCAN algorithm of the
   ClusteringSuite to group the instances according to their duration. */

class InstanceSeparatorDBSCAN : public InstanceSeparator
{
	private:
	const double eps;
	const unsigned minpoints;

	public:
	InstanceSeparatorDBSCAN (unsigned minpoints, double eps,
	  bool keepallgroups);
	unsigned separateInGroups (vector<Instance*> &vi);
	string details(void) const;
	string nameGroup (unsigned) const;
};

#endif /* INSTANCE_SEPARATOR_DBSCAN_H_INCLUDED */
