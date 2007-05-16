ifndef STAPL
 STAPL = $(shell echo "$(PWD)" | sed 's,/include/pAlgorithms/test,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

COLUMN_SORT = $(STAPL)/tools/mtl 

default: test

all: simplepointerjumping_tester srec_tester_slist srec_tester_plist prec_tester sort_tester inner_product_tester

simplepointerjumping_tester: simplepointerjumping_tester.cc
	$(CC) $(CFLAGS) -o simplepointerjumping_tester simplepointerjumping_tester.cc $(LIB)
srec_tester_slist:srec_tester_slist.cc
	$(CC) $(CFLAGS) -o srec_tester_slist srec_tester_slist.cc $(LIB)
srec_tester_plist:srec_tester_plist.cc
	$(CC) $(CFLAGS) -o srec_tester_plist srec_tester_plist.cc $(LIB)
prec_tester: prec_tester.cc
	$(CC) $(CFLAGS) -o prec_tester prec_tester.cc $(LIB)
sort_tester: sort_tester.cc
	$(CC) $(CFLAGS) -O3 -I$(COLUMN_SORT) -o $@ $@.cc $(LIB)
sort_tester_phases: sort_tester.cc
	$(CC) $(CFLAGS) -O3 -D_PHASES_TIMER -I$(COLUMN_SORT) -o $@ sort_tester.cc $(LIB)
inner_product_tester: inner_product_tester.cc
	$(CC) $(CFLAGS) -O3 -o $@ $@.cc $(LIB)

run:
	./testme.sh "$(call staplrun ,4)"

clean:
	rm -rf *.o simplepointerjumping_tester srec_tester_slist srec_tester_plist prec_tester sort_tester inner_product_tester

test: all run clean

DEBUG_FLAGS = -DSTAPL_DEBUG
test-debug: 
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o simplepointerjumping_tester simplepointerjumping_tester.cc $(LIB)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o srec_tester_slist srec_tester_slist.cc $(LIB)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o srec_tester_plist srec_tester_plist.cc $(LIB)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o prec_tester prec_tester.cc $(LIB)
	gmake run clean

#EOF
