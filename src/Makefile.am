SUBDIRS = service-kriger service-R

bin_PROGRAMS = fuse codeblocks extract interpolate foldingtypes
noinst_PROGRAMS = max-diff-interpolation-points mean-diff-interpolation-points

foldingtypes_SOURCES = \
 folding-types.C interpolate.H

codeblocks_SOURCES = \
 common.C common.H \
 codeblocks.C 

fuse_SOURCES = \
 common.C common.H \
 fuse.C 

extract_SOURCES = \
 common.C common.H \
 extract.C \
 interpolate.H

interpolate_SOURCES = \
 common.C common.H \
 sample.C sample.H \
 sample-selector.H \
 sample-selector-first.H sample-selector-first.C sample-selector-distance.H sample-selector-distance.C sample-selector-default.H sample-selector-default.C \
 instance.H instance.C \
 instance-container.C instance-container.H instance-group.C instance-group.H \
 read-extracted-data.C read-extracted-data.H \
 object-selection.H object-selection.C \
 prv-writer.C prv-writer.H \
 interpolation.C interpolation.H \
 interpolation-results.C interpolation-results.H \
 interpolation-kriger.C interpolation-kriger.H \
 interpolation-R-strucchange.C interpolation-R-strucchange.H \
 interpolate.C interpolate.H

if HAVE_CUBE
interpolate_SOURCES += \
 callstack.C callstack.H callstack-tree.C callstack-tree.H \
 cube-holder.C cube-holder.H \
 cube-tree.C cube-tree.H 
endif

max_diff_interpolation_points_SOURCES = max-diff-interpolation-points.c
mean_diff_interpolation_points_SOURCES = mean-diff-interpolation-points.c

fuse_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
codeblocks_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
foldingtypes_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
extract_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
interpolate_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -Iservice-kriger -Iservice-R -g -O
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

fuse_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
codeblocks_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
foldingtypes_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig 
extract_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
interpolate_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser -Lservice-kriger -lkriger -lgfortran -lm -Lservice-R -lR
if HAVE_CUBE
interpolate_LDFLAGS +=  -L$(CUBE_DIR)/lib -lcube3 -lz
endif

max_diff_interpolation_points_LDFLAGS = -lm
mean_diff_interpolation_points_LDFLAGS = -lm
