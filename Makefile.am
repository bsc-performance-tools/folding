SUBDIRS = src doc

ACLOCAL_AMFLAGS=-I config

EXTRA_DIST = others examples cfgs SVN-branch SVN-revision

install-data-hook:
	cp -v others/folding.sh $(prefix)/bin
	cd $(prefix)/bin; $(LN_S) -f folding.sh folding; cd -
	cp -v others/folding-paraver-alienapp.sh $(prefix)/bin
	cd $(prefix)/bin; $(LN_S) -f folding-paraver-alienapp.sh folding-paraver-alienapp; cd -
	cp others/folding-cube-call-paraver.sh $(prefix)/bin
	cp others/folding-gnuplot-compare.py $(prefix)/bin
if WANT_GUI_FRONTEND
	cp others/wxfolding others/wxfolding.py others/wxfolding-viewer others/wxfolding_viewer.py $(prefix)/bin
endif
	$(MKDIR_P) $(prefix)/etc
	cp others/cube-call-paraver-base.cfg $(prefix)/etc
	cp others/breakpoints.R $(prefix)/etc
	cp others/basicblocks.C.py others/basicblocks.F90.py $(prefix)/etc
	cp -r others/models $(prefix)/etc
	cp -r others/extrae-configurations $(prefix)/etc
	$(MKDIR_P) $(datarootdir)
	cp -fr examples $(datarootdir)
	$(MKDIR_P) $(datarootdir)/cfg
	cp -r cfgs/histograms cfgs/views $(datarootdir)/cfg
if LIBTOOLS_HAS_SHARED_LIBRARIES
if !LIBTOOLS_DIR_SAME_AS_LIBDIR
	cp $(LIBTOOLS_LIB_DIR)/libparavertraceconfig.so.0 \
       $(LIBTOOLS_LIB_DIR)/libparavertraceparser.so.0 \
       $(libdir)
endif
	$(LN_S) -f $(libdir)/libparavertraceconfig.so.0 $(libdir)/libparavertraceconfig.so
	$(LN_S) -f $(libdir)/libparavertraceparser.so.0 $(libdir)/libparavertraceparser.so
endif

