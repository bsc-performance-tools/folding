
#include $(top_srcdir)/config/autotroll.mk

SUFFIXES = .moc.cpp .H

.h.moc.cpp:
	$(MOC) $(QT_CPPFLAGS) $(EXTRA_CPPFLAGS) \
    -I$(CUBE_INCLUDE_PATH) -I$(CUBE_GUI_INCLUDE_PATH) \
     $< -o $@

noinst_LTLIBRARIES = libqcustomplot.la

QCUSTOMPLOT_VERSION = 1.3.1

libqcustomplot_la_SOURCES = \
  ${QCUSTOMPLOT_VERSION}/qcustomplot/qcustomplot.cpp \
  ${QCUSTOMPLOT_VERSION}/qcustomplot/qcustomplot.h

# ${QCUSTOMPLOT_VERSION}/qcustomplot/qcustomplot.moc.cpp

libqcustomplot_la_CXXFLAGS = @QT_CXXFLAGS@ @QT_INCPATH@ 

BUILT_SOURCES = ${QCUSTOMPLOT_VERSION}/qcustomplot/qcustomplot.moc.cpp

nodist_libqcustomplot_la_SOURCES = $(BUILT_SOURCES)

DISTCLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = 1.3.1
