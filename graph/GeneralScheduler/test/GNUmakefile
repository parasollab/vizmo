
STAPL = $(shell echo "$(PWD)" | sed 's,/include/GeneralScheduler/test,,')

include $(STAPL)/GNUmakefile.STAPLdefaults

default: all

# clean up
clean: 
	rm -rf *.o core* a.out ii_files rii_files \
	GeneralScheduler_test


# build and run 
all: GeneralScheduler_test

test: all
	./testme.sh "$(call staplrun,1)"

test-debug: all
	./testme.sh "$(call staplrun,1)"

# rules to build the test suites

GeneralScheduler_test: GeneralScheduler_test.cc
	$(CC) -DCHECKSCHEDULE $(USER_OPT) $(CFLAGS) -o $@ $@.cc $(STAPL)/include/GeneralScheduler/Schedule.o $(LIB) 
