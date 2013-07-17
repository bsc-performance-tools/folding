SUBDIRS = src

EXTRA_DIST = others examples

install-data-hook:
	$(SED) "s|FOLDING_HOME=|FOLDING_HOME=$(prefix)|g" < others/fold.sh > $(prefix)/bin/fold.sh
	chmod a+x $(prefix)/bin/fold.sh
	cp others/folding-cube-call-paraver.sh $(prefix)/bin
	cp others/wxfolding others/wxfolding.py others/wxfolding-viewer others/wxfolding_viewer.py $(prefix)/bin
	$(MKDIR_P) $(prefix)/etc
	cp others/cube-call-paraver-base.cfg $(prefix)/etc
	cp others/breakpoints.R $(prefix)/etc
	cp others/basicblocks.C.py others/basicblocks.F90.py $(prefix)/etc
	$(MKDIR_P) $(prefix)/cfg
	cp cfgs/win_folded_caller_types.cfg cfgs/win_folded_types.cfg $(prefix)/cfg
	cp -fr examples $(prefix)

