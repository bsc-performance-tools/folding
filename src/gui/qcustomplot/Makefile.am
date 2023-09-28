
noinst_LTLIBRARIES = libqcustomplot.la
libqcustomplot_la_SOURCES = \
	1.3.1/qcustomplot/qcustomplot.cpp \
	1.3.1/qcustomplot/qcustomplot.h

libqcustomplot_la_CXXFLAGS = @QT_CXXFLAGS@ @QT_INCPATH@
nodist_libqcustomplot_la_SOURCES = 1.3.1/qcustomplot/qcustomplot.moc.cpp
libqcustomplot_la.$(OBJEXT): 1.3.1/qcustomplot/qcustomplot.moc.cpp

CLEANFILES = 1.3.1/qcustomplot/qcustomplot.moc.cpp

SUFFIXES = .moc.cpp .H

.h.moc.cpp:
	$(MOC) $(QT_CPPFLAGS) $(EXTRA_CPPFLAGS) \
	-I$(CUBE_INCLUDE_PATH) -I$(CUBE_GUI_INCLUDE_PATH) \
	$< -o $@

EXTRA_DIST = 1.3.1
