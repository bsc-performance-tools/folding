
SUFFIXES = .moc.cpp .hpp

.hpp.moc.cpp:
	$(MOC) $(QT_CPPFLAGS) $(EXTRA_CPPFLAGS) -I$(CUBE_INCLUDE_PATH) -I$(CUBE_GUI_INCLUDE_PATH) $< -o $@

lib_LTLIBRARIES = cubeplugin-folding-source-code.la

cubeplugin_folding_source_code_la_SOURCES = \
	source-code.cpp source-code.moc.cpp source-code.hpp

cubeplugin_folding_source_code_la_CXXFLAGS = @QT_CXXFLAGS@ @QT_INCPATH@ -I$(CUBE_INCLUDE_PATH) -I$(CUBE_GUI_INCLUDE_PATH)
cubeplugin_folding_source_code_la_LDFLAGS = -shared -avoid-version -module @QT_LIBS@ @CUBE_LDFLAGS@ @CUBE_GUI_LDFLAGS@

BUILT_SOURCES = source-code.moc.cpp

DISTCLEANFILES = $(BUILT_SOURCES)
