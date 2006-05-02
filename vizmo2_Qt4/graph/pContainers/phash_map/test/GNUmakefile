ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/include/pContainers/phash_map/test,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

default: all

all: test_phash

clean:
	rm -rf test_phash *.o *.i *.err

test: all run clean
# Compile the test files
# 
HASH_FLAGS=-I$(STAPL)/include/pContainers/phash_map
test_phash: test_phash.cc
	$(CC) $(CFLAGS) $(HASH_FLAGS) -o $@ $@.cc $(LIB)

run: all
	./testme.sh "$(call staplrun,4)"
