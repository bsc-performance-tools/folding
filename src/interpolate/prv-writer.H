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
#include "common.H"

#ifndef PRV_WRITER_H_INCLUDED
#define PRV_WRITER_H_INCLUDED

#include "ParaverTrace.h"
#include "ParaverTraceThread.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceApplication.h"

#include "instance-container.H"
#include "interpolation-results.H"

#include <fstream>

namespace libparaver {

class FoldedParaverTrace : public ParaverTrace
{
	private:
	ofstream traceout;

	public:
	FoldedParaverTrace (string &traceout, string prvFile, bool multievents);

	void processState (struct state_t &s);
	void processMultiEvent (struct multievent_t &e);
	void processEvent (struct singleevent_t &e);
	void processCommunication (struct comm_t &c);
	void processCommunicator (string &c);
	void processComment (string &c);

	void DumpStartingParaverLine (void);
	void DumpParaverLines (const vector<unsigned long long> &type,
		const vector<unsigned long long > &value, unsigned long long time,
		const Instance *in);

	void DumpParaverLine (unsigned long long type,
		unsigned long long value, unsigned long long time,
	    const Instance *in);

	void DumpGroupInfo (const Instance *in, unsigned PRVfoldedtype);
	bool DumpCallersInInstance (const Instance *in, const InstanceGroup *ig);
	bool DumpAddressesInInstance (const Instance *in, InstanceGroup *ig);
	unsigned DumpReverseCorrectedCallersInInstance (const Instance *in,
	  const InstanceGroup *ig);
	void DumpBreakpoints (const Instance *in, const InstanceGroup *ig);
	void DumpInterpolationData (const Instance *in,
	  const InstanceGroup *ig, map<string, unsigned> counterCodes);
/*
	void DumpReverseMainCallersInInstance (const Instance *in,
	  const InstanceGroup *ig, unsigned mainid);
*/
	void DumpCallstackProcessed (const Instance *in,
	  const InstanceGroup *ig);
};

} /* namespace libparaver */

#endif /* PRV_WRITER_H_INCLUDED */
