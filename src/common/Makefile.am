
noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = \
 common.H common.C
libcommon_la_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
if NEED_BOOST_HOME
libcommon_la_CXXFLAGS += -I$(BOOST_HOME)/include
endif
