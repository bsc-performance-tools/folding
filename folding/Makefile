#DEBUG= -DDEBUG
CXX = g++
BOOST = /home/harald/aplic/boost/1.40
COMMON = ../common-files
COMMONFILESFLAGS = -I $(COMMON) -I $(COMMON)/pcfparser -I $(COMMON)/prvparser -I . -I $(BOOST)/include
CXXFLAGS = -O -Wall -g $(DEBUG)
PRV_PCF_PARSERS = $(COMMON)/prvparser/*.cpp $(COMMON)/pcfparser/*.cpp $(COMMON)/LibException.cpp

TARGETS = fuse sort_pcf extract interpolate

all: $(TARGETS)

translate_prv.o: translate_prv.C
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(COMMONFILESFLAGS)

sort_pcf:	sort_pcf.C translate_prv.o
	$(CXX) -o $@ $< $(PRV_PCF_PARSERS) $(CXXFLAGS) $(COMMONFILESFLAGS) translate_prv.o

common.o: common.C
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(COMMONFILESFLAGS)

fuse: fuse.C
	$(CXX) -o $@ $< $(PRV_PCF_PARSERS) $(CXXFLAGS) $(COMMONFILESFLAGS)

extract: extract.C common.o
	$(CXX) -o $@ extract.C $(PRV_PCF_PARSERS) $(CXXFLAGS) $(COMMONFILESFLAGS) common.o

generate-gnuplot.o: generate-gnuplot.C
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(COMMONFILESFLAGS)

region.o: region.C
	$(CXX) -c -o $@ $< $(CXXFLAGS)

region-analyzer.o: region-analyzer.C
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(COMMONFILESFLAGS)

interpolate: region-analyzer.o region.o generate-gnuplot.o common.o interpolate.C
	cd service-kriger; gmake ; cd ..
	$(CXX) -o $@ interpolate.C $(PRV_PCF_PARSERS) $(CXXFLAGS) $(COMMONFILESFLAGS) -I $(PWD)/service-kriger -L $(PWD)/service-kriger -lkriger -lgfortran region.o region-analyzer.o generate-gnuplot.o common.o

clean:
	cd service-kriger; gmake clean; cd ..
	rm -f *.o $(TARGETS)
