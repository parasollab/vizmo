ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/include/pContainers/pgraph/test/eulertour_tester,,')
endif


all: seulertour_tester peulertour_tester

clean:
	rm -rf *.o seulertour_tester peulertour_tester

include $(STAPL)/GNUmakefile.STAPLdefaults

# Compile the test files
# 
GRAPHFLAGS=-DPARALLEL -D_PGRAPH -DDYNAMIC_GRAPH 

seulertour_tester: seulertour_tester.cc
	$(CC) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)

peulertour_tester: peulertour_tester.cc
	$(CC) $(CFLAGS) $(GRAPHFLAGS) -o $@ $@.cc $(LIB)

