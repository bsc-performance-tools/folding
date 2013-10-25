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

#ifndef INTERPOLATE_H_INCLUDED
#define INTERPOLATE_H_INCLUDED

#define FOLDED_BASE                       600000000
#define FOLDED_INSTANCE_GROUP             600000001
#define FOLDED_PHASE                      600000002
#define FOLDED_TYPE                       600000003
#define PAPI_CHANGE_COUNTER_SET            41999999
#define PAPI_MIN_COUNTER                   42000000
#define PAPI_MAX_COUNTER                   42999999
#define EXTRAE_SAMPLE_CALLER_MIN           30000000
#define EXTRAE_SAMPLE_CALLER_MAX           30000099
#define EXTRAE_SAMPLE_CALLERLINE_MIN       30000100
#define EXTRAE_SAMPLE_CALLERLINE_MAX       30000199
#define EXTRAE_SAMPLE_CALLERLINE_AST_MIN   30000200
#define EXTRAE_SAMPLE_CALLERLINE_AST_MAX   30000299

enum StatisticType_t { STATISTIC_MEAN, STATISTIC_MEDIAN };

#endif /* INTERPOLATE_H_INCLUDED */
