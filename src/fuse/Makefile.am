bin_PROGRAMS = fuse

fuse_SOURCES = fuse.C 

fuse_CXXFLAGS = -I.. -I../common -I$(LIBBSCTOOLS_DIR)/include -g -O 
if NEED_BOOST_HOME
fuse_CXXFLAGS += -I$(BOOST_HOME)/include
endif

fuse_LDADD = -L../common -lcommon -L$(LIBBSCTOOLS_LIB_DIR) -lparavertraceconfig -lparavertraceparser

