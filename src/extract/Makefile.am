bin_PROGRAMS = extract

extract_SOURCES = \
 extract.C \
 prv-semantic.H \
 prv-semantic-CSV.H prv-semantic-CSV.C

extract_CXXFLAGS = -I. -I.. -I../common -I../interpolate -I../io -I$(LIBBSCTOOLS_DIR)/include -g -O
if NEED_BOOST_HOME
extract_CXXFLAGS += -I$(BOOST_HOME)/include
endif

extract_LDADD = -L../common -lcommon -lpcf-common -L$(LIBBSCTOOLS_LIB_DIR) -lparavertraceconfig -lparavertraceparser ../io/libfolding-io-writer.la
