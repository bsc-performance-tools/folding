SUBDIRS = src

EXTRA_DIST = others examples

install-data-hook:
	$(SED) "s|FOLDING_HOME=|FOLDING_HOME=$(prefix)|g" < others/fold.sh > $(prefix)/bin/fold.sh
	chmod a+x $(prefix)/bin/fold.sh
	cp others/cube-call-paraver.sh $(prefix)/bin/cube-call-paraver.sh
	$(MKDIR_P) $(prefix)/etc
	cp others/cube-call-paraver-base.cfg $(prefix)/etc
	cp -fr examples $(prefix)

