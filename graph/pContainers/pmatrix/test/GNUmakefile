ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/include/pContainers/pmatrix/test,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

MATRIXFLAGS = -I$(STAPL)/tools/mtl

default: all

all: test_pmatrix test_pmatrixmult test_matrixmatrixmult test_matrixmatrixmult1 test_matrixmatrixmult2 test_lu test_plu test_plunp

clean:
	rm -rf test_pmatrix test_pmatrixmult test_matrixmatrixmult test_matrixmatrixmult1 test_matrixmatrixmult2 test_lu test_plu test_plunp *.o *.i *.err *~ 

test: all run clean

# Compile the test files
# 

test_pmatrix: test_pmatrix.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(MATRIXFLAGS) -o $@ $@.cc $(LIB)
test_matrixmatrixmult: test_matrixmatrixmult.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(MATRIXFLAGS) -o $@ $@.cc $(LIB)
test_matrixmatrixmult1: test_matrixmatrixmult1.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(MATRIXFLAGS) -o $@ $@.cc $(LIB)
test_matrixmatrixmult2: test_matrixmatrixmult2.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(MATRIXFLAGS) -o $@ $@.cc $(LIB)
test_pmatrixmult: test_pmatrixmult.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(MATRIXFLAGS) -o $@ $@.cc $(LIB)
test_lu: test_lu.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(MATRIXFLAGS) -o $@ $@.cc $(LIB)
test_plu: test_plu.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(MATRIXFLAGS) -o $@ $@.cc $(LIB)
test_plunp: test_plunp.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(MATRIXFLAGS) -o $@ $@.cc $(LIB)

run: all
	./testme.sh "$(call staplrun,4)"
