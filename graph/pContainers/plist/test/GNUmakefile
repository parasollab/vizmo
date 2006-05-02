ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/include/pContainers/plist/test,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

default: all

all: test_plist

clean:
	rm -rf test_plist *.o *.i *.err

test: all run clean

# Compile the test files
# 
LISTFLAGS=-D_PLIST

test_plist: test_plist.cc
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(LISTFLAGS) -o $@ $@.cc $(LIB)

run:
	./testme.sh "$(call staplrun ,4)"
# EOF

