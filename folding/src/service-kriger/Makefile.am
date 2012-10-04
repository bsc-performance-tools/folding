
noinst_LTLIBRARIES = libkriger.la

noinst_PROGRAMS = kriger_test

libkriger_la_SOURCES = \
 def_kinrea.f90 \
 def_kriger.f90 \
 Kriger.f90 \
 kri_openfi.f90 \
 kri_interp.f90 \
 kri_mstock.f90 \
 kri_defgri.f90 \
 kri_defgri.f90 \
 kri_solsys.f90 \
 kri_findpi.f90 \
 kri_covarm.f90 \
 kri_memall.f90 \
 call_Kriger.f90 \
 kriger_wrapper.c kriger_wrapper.h
libkriger_la_CFLAGS = -I.

kriger_test_SOURCES = test.c
kriger_test_LDFLAGS = -lkriger -lgfortran -lm

