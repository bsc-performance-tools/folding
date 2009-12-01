
F77 = gfortran
FFLAGS = -g -O3

MODFILES1= def_kinrea.f90
MODFILES2= def_kriger.f90
SRCFILES = Kriger.f90 kri_openfi.f90 kri_interp.f90 kri_mstock.f90 kri_defgri.f90 kri_defgri.f90 kri_solsys.f90 kri_findpi.f90 kri_covarm.f90 kri_memall.f90 call_Kriger.f90

ALL = libkriger.a 

target: $(ALL)

kriger: def_kinrea.mod def_kriger.mod Kriger.o
	$(F77) $(FFLAGS) -o kriger *.o

libkriger_test:	libkriger.a test.c
	gcc -g -O2 test.c -o libkriger_test -L. -lkriger -lgfortran

libkriger.a:	def_kinrea.mod def_kriger.mod Kriger.o kriger_wrapper.c
	gcc -c -g -O2 kriger_wrapper.c -I.
	ar ruc libkriger.a *.o

Kriger.o:	$(SRCFILES)
	$(F77) -c $(FFLAGS) $(SRCFILES)

def_kinrea.mod:	$(MODFILES1)
	$(F77) -c $(FFLAGS) $(MODFILES1)

def_kriger.mod:	$(MODFILES2)
	$(F77) -c $(FFLAGS) $(MODFILES2)

clean:
	rm -f *.o *.mod kriger libkriger.a libkriger_test
