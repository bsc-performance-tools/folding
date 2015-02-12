
noinst_LTLIBRARIES = libgnuplot.la

libgnuplot_la_SOURCES = \
 generate-gnuplot.C generate-gnuplot.H \
 generate-gnuplot-callstack.C generate-gnuplot-callstack.H \
 generate-gnuplot-references.C generate-gnuplot-references.H
libgnuplot_la_CXXFLAGS = \
 -I. -I.. -I../../common -I../model -I../callstack-processor -I$(LIBTOOLS_DIR)/include $(XML_CPPFLAGS) -O -g
if NEED_BOOST_HOME
libgnuplot_la_CXXFLAGS += -I$(BOOST_HOME)/include
endif
