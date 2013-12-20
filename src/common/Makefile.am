include_HEADERS = sample.H codereftriplet.H

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = \
 common.H common.C \
 object-selection.H object-selection.C \
 sample.H sample.C \
 codereftriplet.H codereftriplet.C

libcommon_la_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
if NEED_BOOST_HOME
libcommon_la_CXXFLAGS += -I$(BOOST_HOME)/include
endif
