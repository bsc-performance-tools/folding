SUBDIRS = src

EXTRA_DIST = others

install-data-hook:
	$(SED) "s|FOLDING_HOME=|FOLDING_HOME=$(prefix)|g" < others/fold.sh > $(prefix)/bin/fold.sh

