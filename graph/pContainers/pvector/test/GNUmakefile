ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/include/pContainers/pvector/test,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

default: all

all: test_pvector

test: all run clean

test_pvector : test_pvector.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) -o $@ $@.cc $(LIB) 

a :a.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS)  -o $@ $@.cc $(LIB)

clean : 
	rm  -rf test_pvector *.o *.i *.err

run:
	./testme.sh "$(call staplrun,2)"

