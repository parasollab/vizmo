ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/include/pContainers/parray/test,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

default: all

all: test_parray

test: all run clean

test_parray: test_parray.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) -o $@ $@.cc $(LIB)

clean:
	rm -rf test_parray *.o *.i *.err

run:
	./testme.sh "$(call staplrun,2)"
# EOF

