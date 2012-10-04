SUBDIRS = service-kriger

bin_PROGRAMS = sort_pcf fuse extract interpolate 
noinst_PROGRAMS = max-diff-interpolation-points mean-diff-interpolation-points

COMMONFILESFLAGS = -I $(HOME)/aplic/libtools/include
CUBE3HOME = /home/harald/aplic/cube/3.5.1

sort_pcf_SOURCES = \
 translate_prv.C translate_prv.H \
 sort_pcf.C

fuse_SOURCES = \
 fuse.C 

extract_SOURCES = \
 common.C common.H \
 extract.C 

interpolate_SOURCES = \
 callstackanalysis.C callstackanalysis.H \
 common.C common.H \
 CubeLaunchFile.C CubeLaunchFile.H \
 generate-gnuplot.C generate-gnuplot.H \
 interpolate.C \
 region-analyzer-first-occurrency.C region-analyzer-first-occurrency.H \
 region-analyzer-in-frame-region.C region-analyzer-in-frame-region.H \
 region-analyzer-in-time-region.C region-analyzer-in-time-region.H \
 region.C region.H\
 treenodeholder.C treenodeholder.H \
 point.H 

max_diff_interpolation_points_SOURCES = max-diff-interpolation-points.c
mean_diff_interpolation_points_SOURCES = mean-diff-interpolation-points.c

sort_pcf_CXXFLAGS = -I. $(COMMONFILESFLAGS)
fuse_CXXFLAGS = -I. $(COMMONFILESFLAGS)
extract_CXXFLAGS = -I. $(COMMONFILESFLAGS)
interpolate_CXXFLAGS = -I. $(COMMONFILESFLAGS) -Iservice-kriger -DCUBE -I/home/harald/aplic/cube/3.5.1/include

sort_pcf_LDFLAGS = -L$(HOME)/aplic/libtools/lib -lparavertraceconfig -lparavertraceparser
fuse_LDFLAGS = -L$(HOME)/aplic/libtools/lib -lparavertraceconfig -lparavertraceparser
extract_LDFLAGS = -L$(HOME)/aplic/libtools/lib -lparavertraceconfig -lparavertraceparser
interpolate_LDFLAGS = -L$(HOME)/aplic/libtools/lib -lparavertraceconfig -lparavertraceparser -L$(CUBE3HOME)/lib -lcube3 -lz -Lservice-kriger -lkriger -lgfortran -lm

max_diff_interpolation_points_LDFLAGS = -lm
mean_diff_interpolation_points_LDFLAGS = -lm
