SUBDIRS = src

EXTRA_DIST = others examples

install-data-hook:
	$(SED) "s|FOLDING_HOME=|FOLDING_HOME=$(prefix)|g" < others/fold.sh > $(prefix)/bin/fold.sh
	chmod a+x $(prefix)/bin/fold.sh
	cp -fr examples $(prefix)

