SUBDIRS = service-kriger service-R common

bin_PROGRAMS = fuse codeblocks extract interpolate foldingtypes
noinst_PROGRAMS = max-diff-interpolation-points mean-diff-interpolation-points

foldingtypes_SOURCES = \
 folding-types.C interpolate.H

codeblocks_SOURCES = \
 codeblocks.C 

fuse_SOURCES = \
 fuse.C 

extract_SOURCES = \
 extract.C \
 prv-semantic.H \
 prv-semantic-CSV.H prv-semantic-CSV.C \
 interpolate.H

interpolate_SOURCES = \
 codereftriplet.H codereftriplet.C codereftriplet-accounting.H codereftriplet-accounting.C \
 sample.C sample.H \
 sample-selector.H \
 sample-selector-first.H sample-selector-first.C sample-selector-distance.H sample-selector-distance.C sample-selector-default.H sample-selector-default.C \
 instance-separator.H instance-separator.C \
 instance-separator-auto.H instance-separator-auto.C instance-separator-none.H instance-separator-none.C \
 instance.H instance.C \
 instance-container.C instance-container.H instance-group.C instance-group.H \
 read-extracted-data.C read-extracted-data.H \
 prv-writer.C prv-writer.H \
 interpolation.C interpolation.H \
 interpolation-results.C interpolation-results.H \
 interpolation-kriger.C interpolation-kriger.H \
 interpolation-R-strucchange.C interpolation-R-strucchange.H \
 interpolate.C interpolate.H \
 model/componentnode.H \
 model/componentnode_constant.C model/componentnode_constant.H \
 model/componentnode_data.C model/componentnode_data.H \
 model/componentnode_derived.C model/componentnode_derived.H \
 model/componentmodel.C model/componentmodel.H \
 model/model.C model/model.H

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

max_diff_interpolation_points_SOURCES = max-diff-interpolation-points.c
mean_diff_interpolation_points_SOURCES = mean-diff-interpolation-points.c

fuse_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -Icommon -g -O 
codeblocks_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -Icommon -g -O
foldingtypes_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -Icommon -g -O
extract_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -Icommon -g -O
interpolate_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -Imodel -Iservice-kriger -Iservice-R -Icommon $(XML_CPPFLAGS) -g -O
if HAVE_CLUSTERING_SUITE
interpolate_CXXFLAGS += -I$(CLUSTERING_HOME)/include
endif
if HAVE_CUBE
interpolate_CXXFLAGS += -I$(CUBE_DIR)/include
endif
if NEED_BOOST_HOME
fuse_CXXFLAGS += -I$(BOOST_HOME)/include
codeblocks_CXXFLAGS += -I$(BOOST_HOME)/include
foldingtypes_CXXFLAGS += -I$(BOOST_HOME)/include
extract_CXXFLAGS += -I$(BOOST_HOME)/include
interpolate_CXXFLAGS += -I$(BOOST_HOME)/include
endif

fuse_LDFLAGS = -Lcommon -lcommon -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
codeblocks_LDFLAGS = -Lcommon -lcommon -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
foldingtypes_LDFLAGS = -Lcommon -lcommon -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig
extract_LDFLAGS = -Lcommon -lcommon -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
interpolate_LDFLAGS = -Lcommon -lcommon -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser -Lservice-kriger -lkriger -lgfortran -lm -Lservice-R -lR -lrt $(XML_LIBS)
if HAVE_CLUSTERING_SUITE
interpolate_LDFLAGS += -L$(CLUSTERING_HOME)/lib -lClustering -lBasicClasses
endif
if HAVE_CUBE
interpolate_LDFLAGS += -L$(CUBE_DIR)/lib -lcube3 -lz
endif

max_diff_interpolation_points_LDFLAGS = -lm
mean_diff_interpolation_points_LDFLAGS = -lm
