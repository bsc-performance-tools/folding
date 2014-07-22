
noinst_LTLIBRARIES = libkriger.la

noinst_PROGRAMS = kriger_test

# Prevent this makefile to be launched in parallel
.NOTPARALLEL:

def_kinrea.mod: def_kinrea.f90
	$(FC) -c $(FCFLAGS) def_kinrea.f90

def_kriger.mod: def_kriger.f90
	$(FC) -c $(FCFLAGS) def_kriger.f90

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
libkriger_la_DEPENDENCIES = def_kinrea.mod def_kinrea.o def_kriger.mod def_kriger.o

kriger_test_SOURCES = test.c
kriger_test_LDFLAGS = -lkriger -lgfortran -lm

