
#include $(top_srcdir)/config/autotroll.mk

SUFFIXES = .moc.cpp .hpp

.hpp.moc.cpp:
	$(MOC) $(QT_CPPFLAGS) $(EXTRA_CPPFLAGS) -I$(CUBE_INCLUDE_PATH) -I$(CUBE_GUI_INCLUDE_PATH) $< -o $@

lib_LTLIBRARIES = cubeplugin-folding-time-line.la

cubeplugin_folding_time_line_la_SOURCES = \
	time-line.cpp time-line.moc.cpp time-line.hpp 

cubeplugin_folding_time_line_la_CXXFLAGS = @QT_CXXFLAGS@ @QT_INCPATH@ -I$(CUBE_INCLUDE_PATH) -I$(CUBE_GUI_INCLUDE_PATH) -I../../qcustomplot/latest/qcustomplot
cubeplugin_folding_time_line_la_LDFLAGS = -shared -avoid-version -module @QT_LIBS@ @CUBE_LDFLAGS@ @CUBE_GUI_LDFLAGS@
cubeplugin_folding_time_line_la_LIBADD  = ../../qcustomplot/libqcustomplot.la 

BUILT_SOURCES = time-line.moc.cpp

DISTCLEANFILES = $(BUILT_SOURCES)
