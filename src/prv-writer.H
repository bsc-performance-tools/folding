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
 | @file: $HeadURL: https://svn.bsc.es/repos/ptools/folding/trunk/src/interpolate.C $
 | 
 | @last_commit: $Date: 2013-05-24 16:08:28 +0200 (dv, 24 mai 2013) $
 | @version:     $Revision: 1764 $
 | 
 | History:
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

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
	void DumpParaverLines (vector<unsigned long long> &type,
		vector<unsigned long long > &value, unsigned long long time,
		unsigned ptask, unsigned task, unsigned thread);

	void DumpParaverLine (unsigned long long type,
		unsigned long long value, unsigned long long time, unsigned ptask,
		unsigned task, unsigned thread);

	void DumpGroupInfo (ObjectSelection *o, Instance *in);
	void DumpCallersInInstance (ObjectSelection *o, Instance *in,
	  InstanceGroup *ig);
	void DumpInterpolationData (ObjectSelection *o, Instance *in,
	  InstanceGroup *ig, map<string, unsigned> counterCodes);
};

} /* namespace libparaver */

#endif /* PRV_WRITER_H_INCLUDED */
