
noinst_LTLIBRARIES = libmodel.la

libmodel_la_SOURCES = \
 componentnode.H componentnode.C \
 componentnode_constant.C componentnode_constant.H \
 componentnode_data.C componentnode_data.H \
 componentnode_derived.C componentnode_derived.H \
 componentmodel.C componentmodel.H \
 model.C model.H 
libmodel_la_CXXFLAGS = \
 -I. -I.. -I../../common -I$(LIBTOOLS_DIR)/include $(XML_CPPFLAGS) -O -g
if NEED_BOOST_HOME
libmodel_la_CXXFLAGS += -I$(BOOST_HOME)/include
endif
