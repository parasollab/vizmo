
STAPL = $(shell echo "$(PWD)" | sed 's,/include/runtime/test,,')

include $(STAPL)/GNUmakefile.STAPLdefaults

default: all

# clean the runtime system and the examples
clean: 
	rm -rf *.o core* a.out ii_files rii_files \
	testArgTransfer testCommPatterns

# build and run all the examples
all: testArgTransfer testCommPatterns 

test: MOREFLAGS = -D_TEST_QUIET
test: all
	./testme.sh "$(call staplrun,4)"

test-debug: MOREFLAGS = -g -DSTAPL_DEBUG
test-debug: all
	./testme.sh "$(call staplrun,4)"

# rules to build the test suites
testArgTransfer: testArgTransfer.cc
	$(CC) $(MOREFLAGS) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)

testCommPatterns: testCommPatterns.cc
	$(CC) $(MOREFLAGS) $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(LIB)

