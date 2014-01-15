SUBDIRS = service-kriger service-R

bin_PROGRAMS = interpolate

interpolate_SOURCES = \
 codereftriplet-accounting.H codereftriplet-accounting.C \
 sample-selector.H \
 sample-selector-first.H sample-selector-first.C \
 sample-selector-distance.H sample-selector-distance.C \
 sample-selector-default.H sample-selector-default.C \
 instance-separator.H instance-separator.C \
 instance-separator-auto.H instance-separator-auto.C \
 instance-separator-none.H instance-separator-none.C \
 instance.H instance.C \
 instance-container.C instance-container.H \
 instance-group.C instance-group.H \
 prv-writer.C prv-writer.H \
 interpolation.C interpolation.H \
 interpolation-results.C interpolation-results.H \
 interpolation-kriger.C interpolation-kriger.H \
 interpolation-R-strucchange.C interpolation-R-strucchange.H \
 model/componentnode.H model/componentnode.C \
 model/componentnode_constant.C model/componentnode_constant.H \
 model/componentnode_data.C model/componentnode_data.H \
 model/componentnode_derived.C model/componentnode_derived.H \
 model/componentmodel.C model/componentmodel.H \
 model/model.C model/model.H \
 generate-gnuplot.C generate-gnuplot.H \
 interpolate.C interpolate.H

if HAVE_CLUSTERING_SUITE
interpolate_SOURCES += \
 instance-separator-dbscan.H instance-separator-dbscan.C
endif
if HAVE_CUBE
interpolate_SOURCES += \
 callstack.C callstack.H callstack-tree.C callstack-tree.H \
 cube-holder.C cube-holder.H \
 cube-tree.C cube-tree.H 
endif

interpolate_CXXFLAGS = -I. -I../common -I../io -I$(LIBTOOLS_DIR)/include -Imodel -Iservice-kriger -Iservice-R $(XML_CPPFLAGS) -g -O 
if HAVE_CLUSTERING_SUITE
interpolate_CXXFLAGS += -I$(CLUSTERING_HOME)/include
endif
if HAVE_CUBE
interpolate_CXXFLAGS += -I$(CUBE_DIR)/include
endif
if NEED_BOOST_HOME
interpolate_CXXFLAGS += -I$(BOOST_HOME)/include
endif

interpolate_LDADD = -L../common -lcommon -lpcf-common -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser -Lservice-kriger -lkriger -lgfortran -lm -Lservice-R -lR -lrt $(XML_LIBS) ../io/libfolding-io-reader.la
if HAVE_CLUSTERING_SUITE
interpolate_LDADD += -L$(CLUSTERING_HOME)/lib -lClustering -lBasicClasses
endif
if HAVE_CUBE
interpolate_LDADD += -L$(CUBE_DIR)/lib -lcube3 -lz
endif
