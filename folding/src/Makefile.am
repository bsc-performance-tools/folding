SUBDIRS = service-kriger

bin_PROGRAMS = sort_pcf fuse extract interpolate 
noinst_PROGRAMS = max-diff-interpolation-points mean-diff-interpolation-points

sort_pcf_SOURCES = \
 translate_prv.C translate_prv.H \
 sort_pcf.C

fuse_SOURCES = \
 fuse.C 

extract_SOURCES = \
 common.C common.H \
 extract.C 

interpolate_SOURCES = \
 common.C common.H \
 generate-gnuplot.C generate-gnuplot.H \
 interpolate.C \
 region-analyzer-first-occurrency.C region-analyzer-first-occurrency.H \
 region-analyzer-in-frame-region.C region-analyzer-in-frame-region.H \
 region-analyzer-in-time-region.C region-analyzer-in-time-region.H \
 region.C region.H region-info.H \
 point.H 
if HAVE_CUBE
interpolate_SOURCES += \
 CubeLaunchFile.C CubeLaunchFile.H \
 treenodeholder.C treenodeholder.H \
 callstackanalysis.C callstackanalysis.H
endif

max_diff_interpolation_points_SOURCES = max-diff-interpolation-points.c
mean_diff_interpolation_points_SOURCES = mean-diff-interpolation-points.c

sort_pcf_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include
fuse_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include
extract_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include
interpolate_CXXFLAGS = -I. -I$(LIBTOOLS_DIR)/include -Iservice-kriger
if HAVE_CUBE
interpolate_CXXFLAGS += -I$(CUBE_DIR)/include
endif

sort_pcf_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
fuse_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
extract_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser
interpolate_LDFLAGS = -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig -lparavertraceparser -Lservice-kriger -lkriger -lgfortran -lm
if HAVE_CUBE
interpolate_LDFLAGS +=  -L$(CUBE_DIR)/lib -lcube3 -lz
endif

max_diff_interpolation_points_LDFLAGS = -lm
mean_diff_interpolation_points_LDFLAGS = -lm
