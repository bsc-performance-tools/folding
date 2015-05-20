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

#include <QVBoxLayout>
#include <QtPlugin>
#include "PluginServices.h"
#include "time-line.hpp"

#include <fstream>
#include <string>
#include <assert.h>

/**  If the plugin doesn't load, the most likely reason is an undefined reference.
     To check this, call "make check-libexample-plugin" in the build-backend directory.
 */

static QVector<QString> activeCounters;
static QString activeRegion;
static unsigned activeGroup = 0;

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( FoldingTimelinePlugin, FoldingTimeline ); // ( PluginName, ClassName )
#endif

class InstantaneousMetrics
{
	private:
	QVector<double> x,y;
	double maxX, maxY;

	public:
	void addValue (double x, double y)
	  {
        if (this->x.size() == 0)
	    { maxX = x; maxY = y; }
	    else
	    { maxX = maxX>x?maxX:x; maxY = maxY>y?maxY:y; }

        this->x.push_back (x); this->y.push_back (y);
	  }
	QVector<double> getXValues(void) const
	  { return x; }
	QVector<double> getYValues(void) const
	  { return y; }
	double getmaxX (void) const
	  { return maxX; }
	double getmaxY (void) const
	  { return maxY; }
};

class RegionGroupCounter
{
	private:
	QString region;
	unsigned group;
	QString counter;

	public:
	RegionGroupCounter (void);
	void setRegion (QString r)
	  { region = r; }
	QString getRegion (void) const
	  { return region; }
	void setGroup (unsigned g)
	  { group = g; }
	unsigned getGroup (void) const
	  { return group; }
	void setCounter (QString c)
	  { counter = c; }
	QString getCounter (void) const
	  { return counter; }
	bool operator<(const RegionGroupCounter & param) const
	{
		if (region == param.region)
		{
			if (group == param.group)
				return counter < param.counter;
			else
				return group < param.group;
		}
		else
			return region < param.region;
	}
};

RegionGroupCounter::RegionGroupCounter ()
{
	region = "";
	group= 0;
	counter = "";
}

static QMap<RegionGroupCounter, InstantaneousMetrics*> RegionGroupCounter_InstantaneousMetrics;

static QVector<QPen> plotPens;
static QPen blackPen;

bool FoldingTimeline::cubeOpened( PluginServices* service )
{
	this->service = service;

	widgetPlot = new QWidget;
	QVBoxLayout* layout = new QVBoxLayout();
	customPlot = new QCustomPlot();
	layout->addWidget (customPlot);
	widgetPlot->setLayout (layout);
	service->addTab (SYSTEM, this);

	QString foldingdatafile = service->getCubeBaseName() + ".slope.csv";

	service->debug() << "loading instantaneous metrics from "
	   << foldingdatafile << endl;

	QFile *fmetrics = new QFile (foldingdatafile);
	if (fmetrics->exists())
	{
		if (fmetrics->open (QIODevice::ReadOnly | QIODevice::Text))
		{
			InstantaneousMetrics *im = new InstantaneousMetrics;
			bool firstread = true;
			RegionGroupCounter previousRGC;
			QString line;

			QTextStream inmetrics (fmetrics);
			while (!inmetrics.atEnd())
			{
				inmetrics >> line;

				if (inmetrics.atEnd())
					break;

				QStringList datacsv = line.split (";");
				assert (datacsv.size() == 5);

				/* Translations done in Folding */
				if (datacsv[2] == "PAPI_TOT_INS")
					datacsv[2] = "MIPS";

				if (firstread)
				{
					previousRGC.setRegion (datacsv[0]);
					previousRGC.setGroup (datacsv[1].toUInt());
					previousRGC.setCounter (datacsv[2]);
					firstread = false;
				}

				/* If we change to another region, group or counter, create new metrics for it */
				if (previousRGC.getRegion() != datacsv[0] ||
				    previousRGC.getGroup() != datacsv[1].toUInt() ||
				    previousRGC.getCounter() != datacsv[2])
				{
					RegionGroupCounter_InstantaneousMetrics[previousRGC] = im;
					previousRGC.setRegion (datacsv[0]);
					previousRGC.setGroup (datacsv[1].toUInt());
					previousRGC.setCounter (datacsv[2]);
					im = new InstantaneousMetrics;
				}

				im->addValue (datacsv[3].toDouble(), datacsv[4].toDouble());
			}
			/* Add last readed data */
			if (!firstread)
				RegionGroupCounter_InstantaneousMetrics[previousRGC] = im;

			fmetrics->close();
		}
		else
		{
			service->debug() << "Cannot open file "
			  << foldingdatafile << " does not exist " << ": "
			  << fmetrics->errorString() << endl;
		}
	}
	else
	{
		service->debug() << "File " << foldingdatafile
		  << " does not exist " << ": " << fmetrics->errorString()
		  << endl;
	}

#define WIDTH_PEN 2
	blackPen.setColor (QColor (0, 0, 0));
	blackPen.setWidthF (WIDTH_PEN);
	QPen p;
	p.setWidthF (WIDTH_PEN);
	p.setColor (QColor (255, 0, 0));
	plotPens.push_back (p);
	p.setColor (QColor (255, 192, 0));
	plotPens.push_back (p);
	p.setColor (QColor (192, 255, 0));
	plotPens.push_back (p);
	p.setColor (QColor (0, 255, 0));
	plotPens.push_back (p);
	p.setColor (QColor (0, 255, 192));
	plotPens.push_back (p);
	p.setColor (QColor (0, 192, 255));
	plotPens.push_back (p);
	p.setColor (QColor (0, 0, 255));
	plotPens.push_back (p);
	p.setColor (QColor (192, 0, 255));
	plotPens.push_back (p);
	p.setColor (QColor (255, 0, 192));
	plotPens.push_back (p);

	activeCounters.push_back ("MIPS");

	connect(
		service, SIGNAL( treeItemIsSelected( TreeType, TreeItem* ) ),
		this, SLOT( treeItemIsSelected( TreeType, TreeItem* ) )
	);

	return true; // initialisation is ok => plugin should be shown
}

void FoldingTimeline::cubeClosed()
{
	delete customPlot;
	delete widgetPlot;
}

/** set a version number, the plugin with the highest version number will be loaded */
void FoldingTimeline::version( int& major, int& minor, int& bugfix ) const
{
	major  = 0;
	minor  = 1;
	bugfix = 0;
}

/** unique plugin name */
QString FoldingTimeline::name() const
{
	return "Folding/TimeLine";
}

QString FoldingTimeline::getHelpText() const
{
	return "No help text available at this moment";
}

/** widget that will be placed into the tab */
QWidget* FoldingTimeline::widget()
{
	return widgetPlot;
}

/** tab label */
QString FoldingTimeline::label() const
{
	return "Instantaneous metrics";
}

/** slot, which is called if a tree item is selected */
void FoldingTimeline::treeItemIsSelected( TreeType type, TreeItem* item )
{
	QString ctrClicked;
	switch (type)
	{
		case METRICTREE:
			ctrClicked = item->getName();
			if (ctrClicked.endsWith ("/ms"))
				ctrClicked.chop(3);
			if (ctrClicked != "MIPS")
			{
				if (!activeCounters.contains(ctrClicked+"_per_ins"))
					activeCounters.push_back (ctrClicked+"_per_ins");
				else
					activeCounters.remove (activeCounters.indexOf(ctrClicked+"_per_ins"));
			}
			else
			{
				if (!activeCounters.contains(ctrClicked))
					activeCounters.push_back (ctrClicked);
				else
					activeCounters.remove (activeCounters.indexOf(ctrClicked));
			}
			service->debug() << "Toggling counter " << ctrClicked << endl;
		break;
		case CALLTREE:
			activeRegion = item->getTopLevelItem()->getName();
		break;
		default:
			return;
		break;
	}

	QString txt = "Showing information for the following counters:";
	QVector<QString>::const_iterator it;
	for (it = activeCounters.constBegin();
	     it != activeCounters.constEnd();
	     ++it)
		txt += *it + " ";
	service->debug() << txt << endl << "Showing information for region '" << activeRegion
	  << "' group = " << activeGroup << endl;

	/* Clean any previous plot */
	customPlot->clearGraphs();

	double Y2max = 1.0f, Ymax = 0.01f;
	bool hasMIPS = false;
	for (it = activeCounters.constBegin();
	     it != activeCounters.constEnd();
	     ++it)
		hasMIPS = *it == "MIPS" || hasMIPS;

	unsigned GraphID = 0;
	unsigned colorCnt = 0;
	for (it = activeCounters.constBegin();
	     it != activeCounters.constEnd();
	     ++it)
	{
		InstantaneousMetrics *im = NULL;
		QString ctr;
		
		QMap<RegionGroupCounter,InstantaneousMetrics*>::const_iterator itm;
		for (itm = RegionGroupCounter_InstantaneousMetrics.constBegin();
		     itm != RegionGroupCounter_InstantaneousMetrics.constEnd();
			 ++itm)
			if (itm.key().getRegion() == activeRegion &&
			    itm.key().getGroup() == activeGroup &&
			    itm.key().getCounter() == *it)
			{
				im = itm.value();
				if (itm.key().getCounter() == "MIPS")
					Y2max = Y2max>im->getmaxY()?Y2max:im->getmaxY();
				else
					Ymax = Ymax>im->getmaxY()?Ymax:im->getmaxY();
				ctr = itm.key().getCounter();
				break;
			}

		if (im != NULL)
		{
			bool isMIPS = ctr=="MIPS";

			QPen pen;
			QCPAxis *ctrAxis;
			if (!isMIPS)
			{
				ctrAxis = customPlot->yAxis;
				pen = plotPens[colorCnt%plotPens.size()];
				colorCnt++;
			}
			else
			{
				ctrAxis = customPlot->yAxis2;
				pen = blackPen;
			}

			customPlot->addGraph(customPlot->xAxis, ctrAxis);
			customPlot->graph(GraphID)->setPen (pen);
			customPlot->graph(GraphID)->setData (im->getXValues(), im->getYValues());
			if (ctr.endsWith ("_per_ins"))
				ctr.chop(QString("_per_ins").length());
			customPlot->graph(GraphID)->setName (ctr);
			GraphID++;
		}
	}

	if (hasMIPS)
	{
		customPlot->yAxis2->setVisible(true);
		customPlot->yAxis2->setRange (0, Y2max);
		customPlot->yAxis2->setLabel ("MIPS");
	}
	else
		customPlot->yAxis2->setVisible(false);

	customPlot->yAxis->setLabel ("Performance counter per instruction");
	customPlot->xAxis->setLabel ("Time (normalized)");
	customPlot->yAxis->setRange (0, Ymax);
	customPlot->xAxis->setRange (0, 1);
	customPlot->legend->setVisible(true);
	customPlot->replot();
}
