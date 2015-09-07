
#include $(top_srcdir)/config/autotroll.mk

SUFFIXES = .moc.cpp .H

.h.moc.cpp:
	$(MOC) $(QT_CPPFLAGS) $(EXTRA_CPPFLAGS) \
    -I$(CUBE_INCLUDE_PATH) -I$(CUBE_GUI_INCLUDE_PATH) \
     $< -o $@

noinst_LTLIBRARIES = libqcustomplot.la

QCUSTOMPLOT_VERSION = 1.3.1

## Newer versions of automake does not seem to support definitions based on
## expanded variables... So be careful on modifying libqcustomplot_la_SOURCES
## and BUILT_SOURCES when upgrading QCUSTOMPLOT

libqcustomplot_la_SOURCES = \
  1.3.1/qcustomplot/qcustomplot.cpp \
  1.3.1/qcustomplot/qcustomplot.h

BUILT_SOURCES = 1.3.1/qcustomplot/qcustomplot.moc.cpp

libqcustomplot_la_CXXFLAGS = @QT_CXXFLAGS@ @QT_INCPATH@ 

nodist_libqcustomplot_la_SOURCES = $(BUILT_SOURCES)

DISTCLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = 1.3.1
