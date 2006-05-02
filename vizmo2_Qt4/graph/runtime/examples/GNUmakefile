
STAPL = ../../..

include $(STAPL)/GNUmakefile.STAPLdefaults

default: all

# clean the runtime system and the examples
clean: 
	rm -rf *.o core* a.out ii_files rii_files \
	helloWorld objectReg innerProduct sampleSort jacobi benchmarkLatency benchmarkFenceLatency benchmarkOverhead testArgTransfer testCommPatterns


# build and run all the examples
all: helloWorld objectReg innerProduct sampleSort jacobi benchmarkLatency benchmarkFenceLatency benchmarkOverhead testArgTransfer testCommPatterns

ifneq ($(rts), MIXED)
runAll: all
	@echo
	$(call staplrun,4) ./helloWorld
	@echo
	$(call staplrun,3) ./objectReg
	@echo
	$(call staplrun,2) ./innerProduct 1000000 3
	@echo
	$(call staplrun,2) ./sampleSort 100000 3
	@echo
	$(call staplrun,2) ./jacobi 64 100 3
	@echo
	$(call staplrun,2) ./benchmarkLatency 10000 100
	@echo
	$(call staplrun,2) ./benchmarkFenceLatency 100
	@echo
	$(call staplrun,1) ./benchmarkOverhead 3
	@echo
	$(call staplrun,3) ./testArgTransfer
	@echo
	$(call staplrun,4) ./testCommPatterns
else
runAll: all
	@echo
	$(call staplrun,2,2) ./helloWorld
	@echo
	$(call staplrun,2,2) ./objectReg
	@echo
#	 $(call staplrun,2) ./innerProduct 1000000 3
#	 @echo
	$(call staplrun,2,2) ./sampleSort 100000 3
	@echo
	$(call staplrun,2,2) ./jacobi 64 100 3
	@echo
	$(call staplrun,1,2) ./benchmarkLatency 10000 100
	@echo
	$(call staplrun,2,1) ./benchmarkLatency 10000 100
	@echo
	$(call staplrun,1,2) ./benchmarkFenceLatency 100
	@echo
	$(call staplrun,2,1) ./benchmarkFenceLatency 100
	@echo
	$(call staplrun,1,1) ./benchmarkOverhead 3
	@echo
	$(call staplrun,2,2) ./testArgTransfer
	@echo
	$(call staplrun,2,2) ./testCommPatterns
endif


# rules to build the examples
helloWorld: helloWorld.cc
	$(CC) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)

objectReg: objectReg.cc
	$(CC) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)

innerProduct: innerProduct.cc
	$(CC) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)

sampleSort: sampleSort.cc
	$(CC) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)

jacobi: jacobi.cc
	$(CC) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)


# rules to build the benchmarks
benchmarkLatency: benchmarkLatency.cc
	$(CC) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)

benchmarkFenceLatency: benchmarkFenceLatency.cc
	$(CC) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)

benchmarkOverhead: benchmarkOverhead.cc
	$(CC) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)


# rules to build the test suites
testArgTransfer: testArgTransfer.cc
	$(CC) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)

testCommPatterns: testCommPatterns.cc
	$(CC) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)
