
# a leaner, meaner makefile that uses defaults defined elsewhere

STAPL=../..

include $(STAPL)/GNUmakefile.STAPLdefaults

default: Schedule.o

Schedule.o: Schedule.cc
	$(CC) $(CFLAGS) -c Schedule.cc 

test: FORCE
	cd test ; gmake test

test-debug: FORCE
	cd test ; gmake test-debug

clean: FORCE
	rm -f Schedule.o
	cd test ; gmake clean

FORCE:


# EOF
