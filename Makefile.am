SUBDIRS = src doc

ACLOCAL_AMFLAGS=-I config

EXTRA_DIST = others examples cfgs 
#SVN-branch SVN-revision

bin_SCRIPTS = others/folding.sh others/folding-paraver-alienapp.sh others/folding-cube-call-paraver.sh others/folding-gnuplot-compare.py
if WANT_GUI_FRONTEND
    bin_SCRIPTS += others/wxfolding others/wxfolding.py others/wxfolding-viewer others/wxfolding_viewer.py
endif

sysconf_DATA = others/cube-call-paraver-base.cfg others/breakpoints.R others/basicblocks.C.py others/basicblocks.F90.py
modelsdir = $(sysconfdir)/models
models_basicdir = $(modelsdir)/basic
models_basic_DATA = others/models/basic/basic-architecture-impact.xml others/models/basic/basic-instruction-mix.xml
models_ibmpower5dir = $(modelsdir)/ibm-power5
models_ibmpower5_DATA = others/models/ibm-power5/powerpc-cpistack-detailed.xml
models_ibmpower7dir = $(modelsdir)/ibm-power7
models_ibmpower7_DATA = others/models/ibm-power7/powerpc-cpistack-detailed.xml
models_ibmpower8dir = $(modelsdir)/ibm-power8
models_ibmpower8_DATA = others/models/ibm-power8/powerpc-cpistack-detailed.xml
models_haswelldir = $(modelsdir)/intel-haswell
models_haswell_DATA = others/models/intel-haswell/intel-hsw-architecture-impact.xml others/models/intel-haswell/intel-hsw-instruction-mix.xml others/models/intel-haswell/intel-hsw-stall-distribution.xml
models_nehalemdir = $(modelsdir)/intel-nehalem
models_nehalem_DATA = others/models/intel-nehalem/intel-nhm-architecture-impact.xml others/models/intel-nehalem/intel-nhm-instruction-mix.xml others/models/intel-nehalem/intel-nhm-stall-distribution.xml
models_sandybridgedir = $(modelsdir)/intel-sandybridge
models_sandybridge_DATA = others/models/intel-sandybridge/intel-snb-architecture-impact.xml others/models/intel-sandybridge/intel-snb-instruction-mix.xml others/models/intel-sandybridge/intel-snb-stall-distribution.xml
models_exynos5dir = $(modelsdir)/samsung-exynos5-armv7l
models_exynos5_DATA = others/models/samsung-exynos5-armv7l/samsung-exynos5-armv7l-architecture-impact.xml others/models/samsung-exynos5-armv7l/samsung-exynos5-armv7l-instruction-mix.xml

extrae_configurationsdir = $(sysconfdir)/extrae-configurations
extrae_configurations_DATA = others/extrae-configurations/extrae-armv7l-samsung-exynos5.xml others/extrae-configurations/extrae-basic.xml others/extrae-configurations/extrae-ibm-power5.xml others/extrae-configurations/extrae-ibm-power7.xml others/extrae-configurations/extrae-ibm-power8.xml others/extrae-configurations/extrae-intel-haswell.xml others/extrae-configurations/extrae-intel-nehalem.xml others/extrae-configurations/extrae-intel-sandybridge.xml

install-exec-hook:
	cd $(bindir); $(LN_S) -f folding.sh folding; cd -
	cd $(bindir); $(LN_S) -f folding-paraver-alienapp.sh folding-paraver-alienapp; cd -

if LIBBSCTOOLS_HAS_SHARED_LIBRARIES
if !LIBBSCTOOLS_DIR_SAME_AS_LIBDIR
	cp $(LIBBSCTOOLS_LIB_DIR)/libparavertraceconfig.so.0 \
       $(LIBBSCTOOLS_LIB_DIR)/libparavertraceparser.so.0 \
       $(libdir)
endif
	cd $(libdir); $(LN_S) -f libparavertraceconfig.so.0 libparavertraceconfig.so; $(LN_S) -f libparavertraceparser.so.0 libparavertraceparser.so
endif
