lib_LTLIBRARIES = libfolding-io-writer.la libfolding-io-common.la
noinst_LTLIBRARIES = libfolding-io-reader.la
include_HEADERS = folding-writer.H sample.H codereftriplet.H

libfolding_io_common_la_SOURCES = \
 sample.C sample.H \
 codereftriplet.C codereftriplet.H

libfolding_io_reader_la_SOURCES = \
 folding-reader.C folding-reader.H

libfolding_io_writer_la_SOURCES = \
 folding-writer.C folding-writer.H

libfolding_io_reader_la_CXXFLAGS = -I. -I.. -I../common -I$(LIBTOOLS_DIR)/include
libfolding_io_writer_la_CXXFLAGS = -I. -I.. -I../common -I$(LIBTOOLS_DIR)/include
libfolding_io_common_la_CXXFLAGS = -I. -I.. -I../common -I$(LIBTOOLS_DIR)/include

