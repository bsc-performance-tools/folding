lib_LTLIBRARIES = libcommon.la
noinst_LTLIBRARIES = libpcf-common.la
include_HEADERS = sample.H codereftriplet.H callstack-codereftriplet.H object-selection.H common.H

libcommon_la_SOURCES = \
 common.H common.C \
 variable-info.H variable-info.C \
 object-selection.H object-selection.C \
 codereftriplet.H codereftriplet.C \
 callstack-codereftriplet.H callstack-codereftriplet.C \
 sample.H sample.C \
 prv-types.H

libpcf_common_la_SOURCES = \
 pcf-common.H pcf-common.C

libcommon_la_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
if NEED_BOOST_HOME
libcommon_la_CXXFLAGS += -I$(BOOST_HOME)/include
endif

libpcf_common_la_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
if NEED_BOOST_HOME
libpcf_common_la_CXXFLAGS += -I$(BOOST_HOME)/include
endif
