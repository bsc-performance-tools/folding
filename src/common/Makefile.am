lib_LTLIBRARIES = libcommon.la
noinst_LTLIBRARIES = libpcf-common.la
include_HEADERS = sample.H codereftriplet.H callstack-codereftriplet.H object-selection.H common.H

libcommon_la_SOURCES = \
 common.H common.C \
 common-math.H common-math.C \
 object-selection.H object-selection.C \
 codereftriplet.H codereftriplet.C \
 callstack-codereftriplet.H callstack-codereftriplet.C \
 data-object.H data-object.C \
 sample.H sample.C \
 prv-colors.H prv-colors.C \
 prv-types.H

libpcf_common_la_SOURCES = \
 pcf-common.H pcf-common.C

libcommon_la_CXXFLAGS = -I. -I$(LIBBSCTOOLS_DIR)/include -g -O
if NEED_BOOST_HOME
libcommon_la_CXXFLAGS += -I$(BOOST_HOME)/include
endif

libpcf_common_la_CXXFLAGS = -I. -I$(LIBBSCTOOLS_DIR)/include -g -O
if NEED_BOOST_HOME
libpcf_common_la_CXXFLAGS += -I$(BOOST_HOME)/include
endif
