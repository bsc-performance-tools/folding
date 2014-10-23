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

#ifndef CODEREFTRIPLET_H_INCLUDED
#define CODEREFTRIPLET_H_INCLUDED

class CodeRefTriplet
{
	private:
	unsigned Caller;
	unsigned CallerLine;
	unsigned CallerLineAST;

	public:
	CodeRefTriplet (void);
	CodeRefTriplet (unsigned Caller, unsigned CallerLine, 
	  unsigned CallerLineAST);

	bool operator==(const CodeRefTriplet &other) const 
	  { return Caller == other.Caller && 
	         CallerLine == other.CallerLine &&
	         CallerLineAST == other.CallerLineAST; }

	bool operator<(const CodeRefTriplet &other) const
	  {
	    if (Caller < other.Caller) 
		return true;
	    else
		return Caller == other.Caller && CallerLine < other.CallerLine;
	  }

	unsigned getCaller(void) const
	  { return Caller; }

	unsigned getCallerLine(void) const
	  { return CallerLine; }

	unsigned getCallerLineAST(void) const
	  { return CallerLineAST; }

	void show (void) const;
};

#endif /* CODEREFTRIPLET_H_INCLUDED */
