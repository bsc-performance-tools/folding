SUBDIRS = src

EXTRA_DIST = others examples cfgs

install-data-hook:
	cp -v others/fold.sh others/folding.sh $(prefix)/bin
	$(SED) -i "s|FOLDING_HOME=|FOLDING_HOME=$(prefix)|g" $(prefix)/bin/fold.sh
	$(SED) -i "s|FOLDING_HOME=|FOLDING_HOME=$(prefix)|g" $(prefix)/bin/folding.sh
	cd $(prefix)/bin; $(LN_S) -f folding.sh folding; cd -
	cp others/folding-cube-call-paraver.sh $(prefix)/bin
if WANT_GUI_FRONTEND
	cp others/wxfolding others/wxfolding.py others/wxfolding-viewer others/wxfolding_viewer.py $(prefix)/bin
endif
	$(MKDIR_P) $(prefix)/etc
	cp others/cube-call-paraver-base.cfg $(prefix)/etc
	cp others/breakpoints.R $(prefix)/etc
	cp others/basicblocks.C.py others/basicblocks.F90.py $(prefix)/etc
	cp -r others/models $(prefix)/etc
	$(MKDIR_P) $(prefix)/cfg
	cp cfgs/win_folded_caller_types.cfg cfgs/win_folded_types.cfg $(prefix)/cfg
	cp -fr examples $(prefix)

