
EXTRA_DIST = \
 user-guide-api.tex \
 user-guide-doc.tex \
 user-guide-get-a-trace.tex \
 user-guide-tool-design.tex \
 user-guide.tex \
 user-guide.pdf \
 user-guide

manual_ug = user-guide

#
# Aquesta combinacio de params pot ser interessant per a generar un single HTML
# -split 0 -show_section_numbers -nonumbered_footnotes -mkdir -dir single

all:
	@echo Nothing to be done by default. If you want to build the documentation issue make build-documentation.

build-documentation: $(manual_ug).tex user-guide-api.tex user-guide-doc.tex user-guide-get-a-trace.tex user-guide-tool-design.tex
	@echo "\newcommand{\FOLDINGVERSION}{$(PACKAGE_VERSION)}" > version.tex
if BUILD_DOCS_PDF
	$(pdflatex) $(manual_ug)
	$(pdflatex) $(manual_ug)
endif
if BUILD_HTML_DOCS
	$(latex2html) -split 0 -show_section_numbers -nonumbered_footnotes $(manual_ug)
	sed s/HREF=\"user-guide.html/HREF=\"/ < user-guide/index.html > user-guide/index.2.html
	sed s/SRC=\"img/SRC=\"http:\\/\\/www.bsc.es\\/sites\\/default\\/files\\/public\\/computer_science\\/performance_tools\\/img/ < user-guide/index.2.html > user-guide/index.bsc.html
	rm -fr user-guide/index.2.html
	cd ..
endif

install-exec-hook: $(manual_ug).pdf
	$(MKDIR_P) $(DESTDIR)$(datadir)/doc
	cp $(manual_ug).pdf $(DESTDIR)$(datadir)/doc
	$(MKDIR_P) $(DESTDIR)$(datadir)/doc/html
	cp user-guide/user-guide.html user-guide/user-guide.css user-guide/*.png $(DESTDIR)$(datadir)/doc/html
	ln -f $(DESTDIR)$(datadir)/doc/html/user-guide.html $(DESTDIR)$(datadir)/doc/html/index.html

clean:
	@echo Nothing to be done by default. If you want to clean the documentation issue make clean-documentation.

clean-documentation:
	rm -fr $(manual_ug).aux $(manual_ug).dvi $(manual_ug).lo[fgt] $(manual_ug).ps $(manual_ug).pdf $(manual_ug).toc $(manual_ug)
