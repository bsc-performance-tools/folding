SUBDIRS = service-kriger service-R model callstack-processor gnuplot

bin_PROGRAMS = interpolate

interpolate_SOURCES = \
 codereftriplet-accounting.H codereftriplet-accounting.C \
 sample-selector.H \
 sample-selector-first.H sample-selector-first.C \
 sample-selector-distance.H sample-selector-distance.C \
 sample-selector-distance-fast.H sample-selector-distance-fast.C \
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

interpolate_CXXFLAGS = -I. -I../common -I../io -I$(LIBBSCTOOLS_DIR)/include -Iservice-kriger -Iservice-R -Imodel -Icallstack-processor -Ignuplot $(XML_CPPFLAGS) -g -O 
if HAVE_CLUSTERING_SUITE
interpolate_CXXFLAGS += -I$(CLUSTERING_HOME)/include
endif
if HAVE_CUBE
interpolate_CXXFLAGS += -I$(CUBE_DIR)/include
endif
if NEED_BOOST_HOME
interpolate_CXXFLAGS += -I$(BOOST_HOME)/include
endif

interpolate_LDADD = ../common/libpcf-common.la \
                    -L$(LIBBSCTOOLS_LIB_DIR) -lparavertraceconfig -lparavertraceparser \
                    service-kriger/libkriger.la \
                    service-R/libR.la \
                    model/libmodel.la \
                    gnuplot/libgnuplot.la \
                    callstack-processor/libcallstack-processor.la \
                    ../io/libfolding-io-reader.la \
                    ../common/libcommon.la \
                    $(XML_LIBS) -lrt -lgfortran -lm
if HAVE_CLUSTERING_SUITE
interpolate_LDADD += -L$(CLUSTERING_HOME)/lib -lClustering -lBasicClasses
endif
if HAVE_CUBE
interpolate_LDADD += -L$(CUBE_DIR_LIB) -lcube4 -lz
endif
