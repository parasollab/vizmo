ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/include/pContainers/pgraph/test/graph_tester,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

all: pgraph_tester sgraph_tester

clean:
	rm -rf *.o pgraph_tester sgraph_tester


# Compile the test files
# 
GRAPHFLAGS=-DPARALLEL -D_PGRAPH -DDYNAMIC_GRAPH 

sgraph_tester: sgraph_tester.cc
	$(CC) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)

pgraph_tester: pgraph_tester.cc
	$(CC) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)
