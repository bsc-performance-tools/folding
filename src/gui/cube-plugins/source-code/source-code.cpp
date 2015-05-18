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
#include "source-code.hpp"

#include <fstream>
#include <string>
#include <assert.h>

#define UNREFERENCED_PARAMETER(a) ((a)=(a))

/**  If the plugin doesn't load, the most likely reason is an undefined reference.
     To check this, call "make check-libexample-plugin" in the build-backend directory.
 */

static QVector<QString> activeCounters;
static TreeItem* activeCallTreeItem = NULL;
static unsigned activeGroup = 0;

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( FoldingSourceCodePlugin, FoldingSourceCode ); // ( PluginName, ClassName )
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

bool FoldingSourceCode::cubeOpened( PluginServices* service )
{
	this->service = service;

	sourceCodeWidget = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout();
	sourceCodeTable = new QTableWidget;
	sourceCodeTable->setEditTriggers (QAbstractItemView::NoEditTriggers);
	sourceCodeTable->verticalHeader()->hide();
	layout->addWidget (sourceCodeTable);
	sourceCodeWidget->setLayout (layout);
	service->addTab( SYSTEM, this );

	QString foldingdata = service->getCubeBaseName() + ".slope.csv";

	service->debug() << "loading instantaneous metrics from "
	   << foldingdata << endl;
	std::ifstream dataf;
	dataf.open (foldingdata.toStdString());
	if (dataf.is_open())
	{
		service->debug() << "file was opened" << endl;

		InstantaneousMetrics *im = new InstantaneousMetrics;

		RegionGroupCounter previousRGC;
		bool firstread = true;

		std::string line;
		dataf >> line;
		while (!dataf.eof())
		{
			QVector<QString> datacsv;

			/* Break each component separated by ; */
			while (line.find (";") != std::string::npos)
			{
				std::string cut = line.substr (0, line.find(";"));
				line = line.substr (line.find (";")+1);
				datacsv.push_back (QString(cut.c_str()));
			}
			/* Add last component */
			datacsv.push_back (QString(line.c_str()));

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

			/* read next line */
			dataf >> line;
		}
		/* Add last readed data */
		if (!firstread)
			RegionGroupCounter_InstantaneousMetrics[previousRGC] = im;
		dataf.close();

		QMap<RegionGroupCounter,InstantaneousMetrics*>::iterator it;
		for (it = RegionGroupCounter_InstantaneousMetrics.begin();
		     it != RegionGroupCounter_InstantaneousMetrics.end();
			 ++it)
			service->debug() << it.key().getRegion() << " " << it.key().getGroup() << " " << it.key().getCounter()  << endl;
	}
	else
		service->debug() << "could not open file" << endl;

	activeCounters.push_back ("MIPS");

	fontSourceCode = QFont("Courier 10 Pitch", 10);

	connect( service, SIGNAL( treeItemIsSelected( TreeType, TreeItem* ) ),
	  this, SLOT( treeItemIsSelected( TreeType, TreeItem* ) ) );
 	connect (sourceCodeTable, SIGNAL( cellClicked (int, int) ),
	  this, SLOT(onTableClick(int,int)));

	return true; // initialisation is ok => plugin should be shown
}

void FoldingSourceCode::cubeClosed()
{
	delete sourceCodeTable;
	delete sourceCodeWidget;
}

/** set a version number, the plugin with the highest version number will be loaded */
void FoldingSourceCode::version( int& major, int& minor, int& bugfix ) const
{
    major  = 0;
    minor  = 1;
    bugfix = 0;
}

/** unique plugin name */
QString FoldingSourceCode::name() const
{
	return "Folding/SourceCode";
}

QString FoldingSourceCode::getHelpText() const
{
	return "No help text available at this moment";
}

/** widget that will be placed into the tab */
QWidget* FoldingSourceCode::widget()
{
	return sourceCodeWidget;
}

/** tab label */
QString FoldingSourceCode::label() const
{
	return "Source-code metrics";
}

/** slot, which is called if a tree item is selected */
void FoldingSourceCode::treeItemIsSelected( TreeType type, TreeItem* item )
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
			activeCallTreeItem = item;
			service->debug() << "Selecting active region from '"
			  << activeCallTreeItem->getTopLevelItem()->getName()
			  << "'" << endl;
		break;
		default:
			return;
		break;
	}

	if (activeCallTreeItem != NULL)
	{
		QString file;
		int startLine, endLine;
		activeCallTreeItem->getSourceInfo (file, startLine, endLine);

		QString txt = "Showing information for the following counters:";
		QVector<QString>::const_iterator it;
		for (it = activeCounters.constBegin();
		     it != activeCounters.constEnd();
		     ++it)
			txt += *it + " ";
		service->debug() << txt << endl;
		service->debug() << "Showing information for region "
		  << activeCallTreeItem->getName() << " group = " << activeGroup
		  << " file selected:: "
		  << file << " between lines " << startLine << "," << endLine
		  << endl;

		fillCodeTable (file, startLine,
		  activeCallTreeItem->getTopLevelItem()->getName(),
		  activeCounters);
	}
}

void FoldingSourceCode::fillCodeTable (const QString & file, int startLine, 
	const QString &topregion, const QVector<QString> & activeCounters)
{
	/* Empty the previous contents of the table */
	for (int i = 0; i < sourceCodeTable->columnCount(); ++i)
		sourceCodeTable->setHorizontalHeaderItem (i, NULL);
	sourceCodeTable->horizontalHeader()->hide();

	while (sourceCodeTable->rowCount() > 0)
		sourceCodeTable->removeRow (0);

	/* Try to open the file */
	QFile *f = new QFile (file);
	if (!f->exists())
	{
		service->debug() << "File " << file << " does not exist "
		  << ": " << f->errorString() << endl;
		delete f;
		return;
	}
	if (!f->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		service->debug() << "Cannot open file " << file
		  << ": " << f->errorString() << endl;
		delete f;
		return;
	}

	/* Create headers */
	/* First refers to phase, then come counters and finally the line & code */
	QVector<QString> metrics;
	metrics.push_back ("Phase");
	for (int i = 0; i < activeCounters.count(); ++i)
		metrics.push_back (activeCounters[i]);
	metrics.push_back ("Line");
	metrics.push_back ("Code");

	sourceCodeTable->setColumnCount (metrics.count());
	for (int i = 0; i < metrics.count(); i++)
	{
		QString ctr = metrics[i];
		if (ctr.endsWith ("_per_ins"))
			ctr.chop(QString("_per_ins").length());
		sourceCodeTable->setHorizontalHeaderItem (i,
		  new QTableWidgetItem (ctr));
	}
	sourceCodeTable->horizontalHeader()->show();

	QFontMetrics fmetric(fontSourceCode);
	for (int i = 0; i < metrics.count()-1; i++)
		sourceCodeTable->setColumnWidth (i,
		  fmetric.boundingRect("W").width() * metrics[i].length());
	sourceCodeTable->setColumnWidth (metrics.count()-1,
	  fmetric.boundingRect("W").width()*80);
	sourceCodeTable->verticalHeader()->setDefaultSectionSize (
	  fmetric.boundingRect("W").height());

	/* Read the source code first. This will limit the number of rows within the table. */
	QTextStream in (f);
	int nlines = 1; 
	while (!in.atEnd())
	{
		QString line = in.readLine();
		QTableWidgetItem *i = new QTableWidgetItem (line);
		i->setFlags (Qt::NoItemFlags | Qt::ItemIsEnabled);
		i->setFont (fontSourceCode);
		sourceCodeTable->insertRow (sourceCodeTable->rowCount());
		sourceCodeTable->setItem (sourceCodeTable->rowCount()-1,
		  sourceCodeTable->columnCount()-1, i);

		QString line_str;
		line_str.setNum (nlines);
		QTableWidgetItem *l = new QTableWidgetItem (line_str);
		l->setFlags (Qt::NoItemFlags | Qt::ItemIsEnabled);
		l->setFont (fontSourceCode);
		l->setTextAlignment (Qt::AlignRight);
		sourceCodeTable->setItem (sourceCodeTable->rowCount()-1,
		  sourceCodeTable->columnCount()-2, l);

		nlines++;
	}

	/* Read the metrics now */
	QString metricfile = file+"."+topregion+".metrics";
	QFile *fmetrics = new QFile (metricfile);
	if (fmetrics->exists())
	{
		if (fmetrics->open (QIODevice::ReadOnly | QIODevice::Text))
		{
			QTextStream inmetrics (fmetrics);
			while (!inmetrics.atEnd())
			{
				unsigned phase;
				QString metric;
				unsigned line;
				double severity;

				inmetrics >> phase;
				if (inmetrics.atEnd())
					break;
				inmetrics >> metric;
				if (inmetrics.atEnd())
					break;
				inmetrics >> line;
				if (inmetrics.atEnd())
					break;
				inmetrics >> severity;
				if (inmetrics.atEnd())
					break;

				putDataintoCodeTable (phase, metric, line,
				  severity, metrics);
			}
			fmetrics->close();
		}
		else
		{
			service->debug() << "Cannot open file " << metricfile
			  << " does not exist " << ": "
			  << fmetrics->errorString() << endl;
		}
	}
	else
	{
		service->debug() << "File " << metricfile << " does not exist "
		  << ": " << fmetrics->errorString() << endl;
	}

	delete fmetrics;

	f->close();
	delete f;

	sourceCodeTable->resizeColumnsToContents();
	sourceCodeTable->scrollToItem (
	  sourceCodeTable->item (startLine, 0),
	  QAbstractItemView::PositionAtCenter );
}

void FoldingSourceCode::putDataintoCodeTable (unsigned phase, 
	const QString &metric, unsigned line, double severity,
	const QVector<QString> &metrics)
{
	int column;

	if ((column = metrics.indexOf (metric)) != -1)
	{
		QString phase_str, severity_str;
		phase_str.setNum (phase);
		severity_str.setNum (severity, 'f', 3);

		QTableWidgetItem *previtemPhase = sourceCodeTable->item (line-1, 0);
		if (previtemPhase == NULL)
		{
			sourceCodeTable->setItem (line-1, 0,
			  new QTableWidgetItem());
			sourceCodeTable->item (line-1, 0)->setFont (fontSourceCode);
			sourceCodeTable->item (line-1, 0)->setText (phase_str);
		}
		else
		{
			QString tmp_str = previtemPhase->text();
			tmp_str += "," + phase_str;
			previtemPhase->setText (tmp_str);
		}

		QTableWidgetItem *previtemMetric = sourceCodeTable->item (line-1,
		  column);
		if (previtemMetric == NULL)
		{
			sourceCodeTable->setItem (line-1, column,
			  new QTableWidgetItem());
			sourceCodeTable->item (line-1, column)->setFont(fontSourceCode);
			sourceCodeTable->item (line-1, column)->setText(severity_str);
		}
		else
		{
			QString tmp_str = previtemMetric->text();
			tmp_str += "," + severity_str;
			previtemMetric->setText (tmp_str);
		}
	}
}

void FoldingSourceCode::onTableClick (int row, int column)
{
	UNREFERENCED_PARAMETER(row);

	double min = DBL_MAX;
	double max = 0;

	QColor white(255,255,255);
	QColor black(0,0,0);
	for (int i = 0; i < sourceCodeTable->rowCount(); i++)
		for (int j = 0; j < sourceCodeTable->columnCount(); j++)
		{
			QTableWidgetItem *item = sourceCodeTable->item (i, j);
			if (item != NULL)
			{
				item->setBackgroundColor (white);
				item->setTextColor (black);
			}
		}

	if (column >= sourceCodeTable->columnCount()-2)
		return;

	QMap<unsigned, double> MaxValueInLine;
	for (int i = 0; i < sourceCodeTable->rowCount(); i++)
	{
		QTableWidgetItem *item = sourceCodeTable->item (i, column);
		if (item != NULL)
		{
			QString s = item->text();
			if (s.length() > 0)
			{
				QStringList items = s.split (",");
				double maxvalline = 0.0f;
				if (MaxValueInLine.count (i) > 0)
					maxvalline = MaxValueInLine[i];
				for (int i = 0; i < items.size(); ++i)
				{
					double value = items[i].toFloat();
					max = (max > value) ? max : value;
					min = (min < value) ? min : value;
					maxvalline = (maxvalline > value) ? maxvalline : value;
				}
				MaxValueInLine[i] = maxvalline;
			}
		}
	}

	QMap<unsigned, double>:: iterator it;
	for (it = MaxValueInLine.begin(); it != MaxValueInLine.end(); it++)
	{
		QColor bgcolor = service->getColor(it.value(), min, max);

		/* From http://en.wikipedia.org/wiki/Luminance_(relative) */
		qreal bgluma =
		  bgcolor.redF() * 0.2126 + 
		  bgcolor.greenF() * 0.7152 + 
		  bgcolor.blueF() * 0.0722;
				
		QColor fgcolor = (bgluma > 0.5) ? black : white;

		for (int j = 0; j < sourceCodeTable->columnCount(); j++)
		{
			QTableWidgetItem *item = sourceCodeTable->item (
			  it.key(), j);
			if (item != NULL)
			{
				item->setBackgroundColor (bgcolor);
				item->setTextColor (fgcolor);
			}
			else
			{
				QTableWidgetItem *tmp = new QTableWidgetItem;
				tmp->setBackgroundColor (bgcolor);
				tmp->setTextColor (fgcolor);
				tmp->setFont(fontSourceCode);
				sourceCodeTable->setItem (it.key(), j, tmp);
			}
		}
	}
}

