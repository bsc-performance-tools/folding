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

#ifndef DATAOBJECT_H_INCLUDED
#define DATAOBJECT_H_INCLUDED

class DataObject
{
	private:
	const unsigned long long size;
	const string name;
	unsigned long long startAddress;

	public:
	DataObject (unsigned long long s, const string & n);
	void setStartAddress (unsigned long long startAddress)
	  { this->startAddress = startAddress; }
	unsigned long long getSize(void) const
	  { return size; }
	unsigned long long getStartAddress (void) const
	  { return startAddress; }
	unsigned long long getEndAddress (void) const
	  { return startAddress + size; }
	const string getName (void) const
	  { return name; }
	bool addressInVariable (unsigned long long address) const
	  { return startAddress <= address && address <= startAddress+size-1; }

	virtual ~DataObject () = 0; /* Force abstract class */
};

class DataObject_static : public DataObject
{
	public:
	DataObject_static (unsigned long long sa, unsigned long long ea,
	  const string &varname);
};

class DataObject_dynamic : public DataObject
{
	public:
	DataObject_dynamic (unsigned long long s, const string & varname);
	DataObject_dynamic (unsigned long long sa, unsigned long long ea,
	  const string &varname);
};

#endif
