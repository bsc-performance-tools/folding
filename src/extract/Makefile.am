bin_PROGRAMS = extract

extract_SOURCES = \
 extract.C \
 prv-semantic.H \
 prv-semantic-CSV.H prv-semantic-CSV.C

extract_CXXFLAGS = -I. -I.. -I../common -I../interpolate -I../io -I$(LIBTOOLS_DIR)/include -g -O
if NEED_BOOST_HOME
extract_CXXFLAGS += -I$(BOOST_HOME)/include
endif

extract_LDADD = -L../common -lcommon -lpcf-common -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser ../io/libfolding-io-writer.la
