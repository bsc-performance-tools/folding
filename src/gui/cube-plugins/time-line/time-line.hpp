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

#ifndef FOLDING_TIMELINE_PLUGIN_H
#define FOLDING_TIMELINE_PLUGIN_H

#include <QWidget>
#include "PluginServices.h"
#include "CubePlugin.h"
#include "TabInterface.h"

#include "qcustomplot.h"

class FoldingTimeline: public QObject, public CubePlugin, TabInterface
{
    Q_OBJECT
    Q_INTERFACES( CubePlugin )
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA( IID "FoldingTimelinePlugin" ) // unique plugin name
#endif

public:
    // CubePlugin implementation
    virtual bool cubeOpened( PluginServices* service );
    virtual void cubeClosed();
    virtual QString name() const;
    virtual void version( int& major, int& minor, int& bugfix ) const;
    virtual QString getHelpText() const;

    // TabInterface implementation
    virtual QString label() const;
    virtual QWidget* widget();

private slots:
    void treeItemIsSelected( TreeType  type, TreeItem* item );

private:
    QCustomPlot * customPlot;
	QWidget * widgetPlot;
    PluginServices* service;

    // TabInterface interface
};

#endif // FOLDING_TIMELINE_PLUGIN_H
