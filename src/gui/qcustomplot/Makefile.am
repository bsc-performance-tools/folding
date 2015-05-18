
#include $(top_srcdir)/config/autotroll.mk

SUFFIXES = .moc.cpp .H

.h.moc.cpp:
	$(MOC) $(QT_CPPFLAGS) $(EXTRA_CPPFLAGS) -I$(CUBE_INCLUDE_PATH) -I$(CUBE_GUI_INCLUDE_PATH) $< -o $@

noinst_LTLIBRARIES = libqcustomplot.la

QCUSTOMPLOT_VERSION = latest

libqcustomplot_la_SOURCES = \
  ${QCUSTOMPLOT_VERSION}/qcustomplot/qcustomplot.cpp ${QCUSTOMPLOT_VERSION}/qcustomplot/qcustomplot.moc.cpp ${QCUSTOMPLOT_VERSION}/qcustomplot/qcustomplot.h

libqcustomplot_la_CXXFLAGS = @QT_CXXFLAGS@ @QT_INCPATH@ 

BUILT_SOURCES = ${QCUSTOMPLOT_VERSION}/qcustomplot/qcustomplot.moc.cpp

DISTCLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = latest 1.3.1
