lib_LTLIBRARIES = libfolding-io-writer.la
noinst_LTLIBRARIES = libfolding-io-reader.la
include_HEADERS = folding-writer.H

libfolding_io_reader_la_SOURCES = \
 folding-reader.C folding-reader.H

libfolding_io_writer_la_SOURCES = \
 folding-writer.C folding-writer.H

libfolding_io_reader_la_CXXFLAGS = -I. -I.. -I../common -I../interpolate -I$(LIBBSCTOOLS_DIR)/include
libfolding_io_writer_la_CXXFLAGS = -I. -I.. -I../common -I$(LIBBSCTOOLS_DIR)/include

if NEED_BOOST_HOME
libfolding_io_reader_la_CXXFLAGS += -I$(BOOST_HOME)/include
libfolding_io_writer_la_CXXFLAGS += -I$(BOOST_HOME)/include
endif
