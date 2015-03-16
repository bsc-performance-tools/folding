FOLDING_HOME=../../..

ALL = folding-writer

TARGET: $(ALL)

folding-writer: folding-writer.C
	g++ folding-writer.C -I $(FOLDING_HOME)/include -L $(FOLDING_HOME)/lib -lfolding-io-writer -lcommon -o folding-writer

clean:
	rm -f $(ALL)

