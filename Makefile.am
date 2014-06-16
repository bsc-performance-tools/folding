SUBDIRS = src doc

EXTRA_DIST = others examples cfgs SVN-branch SVN-revision

install-data-hook:
	cp -v others/folding.sh $(prefix)/bin
	cd $(prefix)/bin; $(LN_S) -f folding.sh folding; cd -
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
	$(MKDIR_P) $(prefix)/cfg
	cp cfgs/win_folded_caller_types.cfg cfgs/win_folded_types.cfg $(prefix)/cfg
	cp -fr examples $(prefix)
if LIBTOOLS_HAS_SHARED_LIBRARIES
	cp $(LIBTOOLS_LIB_DIR)/libparavertraceconfig.so.0 $(LIBTOOLS_LIB_DIR)/libparavertraceparser.so.0 $(libdir)
	$(LN_S) -f $(libdir)/libparavertraceconfig.so.0 $(libdir)/libparavertraceconfig.so
	$(LN_S) -f $(libdir)/libparavertraceparser.so.0 $(libdir)/libparavertraceparser.so
endif

