
noinst_LTLIBRARIES = libcallstack-processor.la

libcallstack_processor_la_SOURCES = \
 callstack-processor.C callstack-processor.H \
 callstack-processor-consecutive-recursive.C callstack-processor-consecutive-recursive.H \
 callstack-processor-consecutive-recursive-ctrl.C callstack-processor-consecutive-recursive-ctrl.H
libcallstack_processor_la_CXXFLAGS = \
 -I. -I.. -I../model -I../../common -I$(LIBBSCTOOLS_DIR)/include $(XML_CPPFLAGS) -O -g
if HAVE_CUBE
libcallstack_processor_la_CXXFLAGS += -I$(CUBE_DIR)/include
endif
if NEED_BOOST_HOME
libcallstack_processor_la_CXXFLAGS += -I$(BOOST_HOME)/include
endif
