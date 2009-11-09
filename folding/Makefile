CXX = g++
BOOST = /home/harald/aplic/boost/1.40
COMMON = ../common-files
COMMONFILESFLAGS = -I $(COMMON) -I $(COMMON)/pcfparser -I $(COMMON)/prvparser -I .
CXXFLAGS = -O -Wall -g
PRV_PCF_PARSERS = $(COMMON)/prvparser/*.cpp $(COMMON)/pcfparser/*.cpp $(COMMON)/LibException.cpp

TARGETS = fuse extract interpolate

all: $(TARGETS)

fuse: fuse.cpp
	$(CXX) -o $@ $< $(PRV_PCF_PARSERS) $(CXXFLAGS) $(COMMONFILESFLAGS) -I $(BOOST)/include

extract: extract.cpp
	$(CXX) -o $@ $< $(PRV_PCF_PARSERS) $(CXXFLAGS) $(COMMONFILESFLAGS) -I $(BOOST)/include

generate-gnuplot.o: generate-gnuplot.cpp
	$(CXX) -O -c -o $@ $<

region.o: region.cpp
	$(CXX) -O -c -o $@ $<

region-analyzer.o: region-analyzer.cpp
	$(CXX) -O -c -o $@ $< $(CXXFLAGS) $(COMMONFILESFLAGS) -I $(BOOST)/include

interpolate: region-analyzer.o region.o generate-gnuplot.o interpolate.cpp
	cd service-kriger; gmake ; cd ..
	$(CXX) -o $@ interpolate.cpp $(PRV_PCF_PARSERS) $(CXXFLAGS) $(COMMONFILESFLAGS) -I $(BOOST)/include -I $(PWD)/service-kriger -L $(PWD)/service-kriger -lkriger -lgfortran region.o region-analyzer.o generate-gnuplot.o

clean:
	cd service-kriger; gmake clean; cd ..
	rm -f *.o $(TARGETS)
