
CXX = g++
BOOST_HOME=/usr/local
CXXFLAGS = -O2 -W -Wall -g -I../common-files/prvparser -I../common-files -I$(BOOST_HOME)/include -I. -I../common-files/pcfparser

all: folding sort_pcf

folding: main.C FunctionBursts.C TimePartition.C
	cd service-kriger; gmake ; cd ..
	$(CXX) main.C FunctionBursts.C TimePartition.C ../common-files/LibException.cpp ../common-files/prvparser/*.cpp ../common-files/pcfparser/*.cpp $(CXXFLAGS) -Iservice-kriger -Lservice-kriger -lkriger -lgfortran -o folding

sort_pcf:
	$(CXX) -Wall -Wextra sort_pcf.C -I ../common-files/pcfparser  ../common-files/pcfparser/*.cpp -o sort_pcf

clean:
	rm -fr folding sort_pcf
	cd service-kriger; gmake clean ; cd ..

