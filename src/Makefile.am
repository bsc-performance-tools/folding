SUBDIRS = service-kriger

bin_PROGRAMS = fuse codeblocks extract interpolate 
noinst_PROGRAMS = max-diff-interpolation-points mean-diff-interpolation-points

codeblocks_SOURCES = \
 common.C common.H \
 codeblocks.C 

fuse_SOURCES = \
 fuse.C 

extract_SOURCES = \
 common.C common.H \
 extract.C 

interpolate_SOURCES = \
 common.C common.H \
 sample.C sample.H \
 read-extracted-data.C read-extracted-data.H \
 point.H object-selection.H object-selection.C \
 instance.H instance.C instance-container.C instance-container.H instance-group.C instance-group.H \
 prv-writer.C prv-writer.H \
 sample-selector.H sample-selector-first.H sample-selector-first.C sample-selector-distance.H sample-selector-distance.C sample-selector-default.H sample-selector-default.C \
 interpolation.C interpolation.H interpolation-results.C interpolation-results.H interpolation-kriger.C interpolation-kriger.H \
 interpolate.C

if HAVE_CUBE
interpolate_SOURCES += \
 CubeLaunchFile.C CubeLaunchFile.H \
 treenodeholder.C treenodeholder.H \
 callstackanalysis.C callstackanalysis.H
endif

max_diff_interpolation_points_SOURCES = max-diff-interpolation-points.c
mean_diff_interpolation_points_SOURCES = mean-diff-interpolation-points.c

fuse_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
codeblocks_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
extract_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -g -O
interpolate_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -Iservice-kriger -g -O
if HAVE_CUBE
interpolate_CXXFLAGS += -I$(CUBE_DIR)/include
endif
if NEED_BOOST_HOME
fuse_CXXFLAGS += -I$(BOOST_HOME)/include
codeblocks_CXXFLAGS += -I$(BOOST_HOME)/include
extract_CXXFLAGS += -I$(BOOST_HOME)/include
interpolate_CXXFLAGS += -I$(BOOST_HOME)/include
endif

fuse_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
codeblocks_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
extract_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
interpolate_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser -Lservice-kriger -lkriger -lgfortran -lm
if HAVE_CUBE
interpolate_LDFLAGS +=  -L$(CUBE_DIR)/lib -lcube3 -lz
endif

max_diff_interpolation_points_LDFLAGS = -lm
mean_diff_interpolation_points_LDFLAGS = -lm
