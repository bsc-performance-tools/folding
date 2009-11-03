
CXX = g++
BOOST_HOME = /usr/local
COMMON_FILES_DIR = ../../common-files
CXXFLAGS = -O2 -W -Wall -g -I$(COMMON_FILES_DIR)/prvparser -I$(COMMON_FILES_DIR)/pcfparser -I$(COMMON_FILES_DIR) -I$(BOOST_HOME)/include -I.

SHARED_C=threadinformation.C taskinformation.C informationholder.C globals.C process.C
SHARED_H=threadinformation.H taskinformation.H informationholder.H globals.H process.H
SHARED_O=threadinformation.o taskinformation.o informationholder.o globals.C process.o

all: sort_pcf analyze-phases compute-phases

informationholder.o: informationholder.C informationholder.H
	$(CXX) -c $(CXXFLAGS) informationholder.C

globals.o: globals.C globals.H
	$(CXX) -c $(CXXFLAGS) globals.C

process.o: process.C process.H
	$(CXX) -c $(CXXFLAGS) process.C

threadinformation.o: threadinformation.C threadinformation.H
	$(CXX) -c $(CXXFLAGS) threadinformation.C

taskinformation.o: taskinformation.C taskinformation.H
	$(CXX) -c $(CXXFLAGS) taskinformation.C

analyze-phases: analyze-phases.C FunctionBursts.C TimePartition.C threadinformation.o taskinformation.o process.o globals.o informationholder.o $(SHARED_H)
	$(CXX) analyze-phases.C FunctionBursts.C TimePartition.C $(SHARED_O) $(COMMON_FILES_DIR)/LibException.cpp $(COMMON_FILES_DIR)/prvparser/*.cpp $(COMMON_FILES_DIR)/pcfparser/*.cpp $(CXXFLAGS) -o $@

compute-phases: compute-phases.C FunctionBursts.C TimePartition.C threadinformation.o taskinformation.o process.o globals.o informationholder.o $(SHARED_H)
	cd service-kriger; gmake ; cd ..
	$(CXX) compute-phases.C FunctionBursts.C TimePartition.C $(SHARED_O) $(COMMON_FILES_DIR)/LibException.cpp $(COMMON_FILES_DIR)/prvparser/*.cpp $(COMMON_FILES_DIR)/pcfparser/*.cpp $(CXXFLAGS) -Iservice-kriger -Lservice-kriger -lkriger -lgfortran -o $@

sort_pcf:
	$(CXX) -Wall -Wextra sort_pcf.C -I $(COMMON_FILES_DIR)/pcfparser $(COMMON_FILES_DIR)/pcfparser/*.cpp -o sort_pcf

clean:
	rm -fr *.o analyze-phases compute-phases sort_pcf
	cd service-kriger; gmake clean ; cd ..

