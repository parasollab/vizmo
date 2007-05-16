ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/include/pContainers/pgraph/test,,')
endif


include $(STAPL)/GNUmakefile.STAPLdefaults

default: all

#08feb2005/timmie - Removed test_partition because it isn't compiling on AIX.
#all: test_pgraph test_pbasegraph test_setget test_pgraph1 test_adddel test_vertexmethods test_directed test_undirected test_pgraphalgo test_partition

all: test_pgraph test_pbasegraph test_setget test_pgraph1 test_adddel test_vertexmethods test_directed test_undirected test_pgraphalgo test_partition

clean:
	rm -rf *.o test_adddel test_pbasegraph test_setget test_pgraph test_pgraph1 crude* dummy* *~ pgout* test_vertexmethods test_directed test_undirected test_pgraphalgo test_partition

test: all run clean

# Compile the test files
# 
GRAPHFLAGS=-DPARALLEL -D_PGRAPH -DDYNAMIC_GRAPH

test_pgraph: test_pgraph.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)
test_pbasegraph: test_pbasegraph.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)
test_setget: test_setget.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)
test_pgraph1: test_pgraph1.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)
test_adddel: test_adddel.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)
test_vertexmethods: test_vertexmethods.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)
test_directed: test_directed.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)
test_undirected: test_undirected.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)
test_pgraphalgo: test_pgraphalgo.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)
test_partition: test_partition.cc
	$(CC) $(DEBUG_FLAGS) -I$(STAPL)/tools/metis/Lib $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB) $(STAPL)/lib/libmetis.a

run: 
	#run the individual tests
	./testme.sh "$(call staplrun,2)"
