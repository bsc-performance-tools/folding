bin_PROGRAMS = codeblocks

codeblocks_SOURCES = codeblocks.C 

codeblocks_CXXFLAGS = -I.. -I../common -I$(LIBBSCTOOLS_DIR)/include -g -O
if NEED_BOOST_HOME
codeblocks_CXXFLAGS += -I$(BOOST_HOME)/include
endif

codeblocks_LDADD = ../common/libcommon.la ../common/libpcf-common.la \
                   -L$(LIBBSCTOOLS_LIB_DIR) -lparavertraceconfig -lparavertraceparser
