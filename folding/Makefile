CXX = g++
BOOST = /home/harald/aplic/boost/1.40
COMMON = ../common-files
COMMONFILESFLAGS = -I $(COMMON) -I $(COMMON)/pcfparser -I $(COMMON)/prvparser
CXXFLAGS = -O -Wall 

all: fuse extract interpolate

fuse: fuse.cpp
	$(CXX) -o $@ $< $(COMMON)/prvparser/*.cpp $(COMMON)/pcfparser/*.cpp $(COMMON)/LibException.cpp $(CXXFLAGS) $(COMMONFILESFLAGS) -I$(BOOST)/include

extract: extract.cpp
	$(CXX) -o $@ $< $(COMMON)/prvparser/*.cpp $(COMMON)/pcfparser/*.cpp $(COMMON)/LibException.cpp $(CXXFLAGS) $(COMMONFILESFLAGS) -I$(BOOST)/include

interpolate: interpolate.cpp
	cd service-kriger; gmake ; cd ..
	$(CXX) -o $@ $< $(CXXFLAGS) -I $(PWD)/service-kriger -L $(PWD)/service-kriger -lkriger -lgfortran

clean:
	cd service-kriger; gmake clean; cd ..
	rm extract interpolate fuse
