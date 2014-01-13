bin_PROGRAMS = foldingtypes
noinst_PROGRAMS = max-diff-interpolation-points mean-diff-interpolation-points

foldingtypes_SOURCES = folding-types.C 

max_diff_interpolation_points_SOURCES = max-diff-interpolation-points.c
mean_diff_interpolation_points_SOURCES = mean-diff-interpolation-points.c

foldingtypes_CXXFLAGS = -I.. -I../common -I../interpolate -I$(LIBTOOLS_DIR)/include -g -O

foldingtypes_LDADD = -L../common -lcommon -L$(LIBTOOLS_DIR)/lib -lparavertraceconfig

max_diff_interpolation_points_LDADD = -lm
mean_diff_interpolation_points_LDADD = -lm
