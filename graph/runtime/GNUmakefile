
# new approach to Makefiles

ifndef STAPL
  STAPL = ../..
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

default: $(PLATFORMDEPS) $(STAPL)/lib/runtime.o $(STAPL)/lib/timer.o

# The basic targets that may be specified by the user:
runtime.o: $(STAPL)/lib/runtime.o $(PLATFORMDEPS)
timer.o: $(STAPL)/lib/timer.o $(PLATFORMDEPS)

$(STAPL)/lib/timer.o: timer.cc
	$(CC) $(MOREFLAGS) $(USER_OPT) $(CFLAGS) -o $@ -c $< 

clean: rts_clean

rts_clean:
	rm -rf $(STAPL)/lib/runtime.o $(STAPL)/lib/primitives.o $(STAPL)/lib/runtime.a $(STAPL)/lib/timer.o

# The runtime system is compiled one component at a time, then linked into
# a single runtime.o object file.
$(STAPL)/lib/runtime.o: $(STAPL)/GNUmakefile.STAPLdefaults $(STAPL)/lib/primitives.o
	$(LD) $(STAPL)/lib/primitives.o -o $(STAPL)/lib/runtime.o
	$(AR) $(STAPL)/lib/runtime.a $(STAPL)/lib/primitives.o


rtslower=$(shell echo $(rts) | tr [:upper:] [:lower:])
$(STAPL)/lib/primitives.o: $(STAPL)/GNUmakefile.$(rts) $(STAPL)/include/runtime/primitives.h $(STAPL)/include/runtime/$(rtslower)_primitives.h $(STAPL)/include/runtime/$(rtslower)_primitives.cc
	$(CC) $(RTS_OPT) $(CFLAGS) -o $(STAPL)/lib/primitives.o -c $(STAPL)/include/runtime/$(rtslower)_primitives.cc

test: $(PLATFORMDEPS) $(STAPL)/lib/runtime.o $(STAPL)/lib/timer.o timer
	$(call staplrun,1) timer
	@cd test; gmake test; cd ..

test-debug: $(PLATFORMDEPS) $(STAPL)/lib/runtime.o $(STAPL)/lib/timer.o
	@cd test; gmake test-debug; cd ..

timer: timer.cc
	$(CC) $(MOREFLAGS) $(USER_OPT) $(CFLAGS) -o $@ $< $(LIB) -DSTAPL_SELF_TEST 

#timers: timer_clock_gettime timer_gettimeofday timer_getrusage timer_papi 
#	@echo $@ built
#timer_clock_gettime: timer.cc timer.h
#	$(CC) $(MOREFLAGS) $(USER_OPT) $(CFLAGS) -o $@ $< $(LIB) -DSTAPL_SELF_TEST -DSTAPL_TIMER_CLOCK_GETTIME
#timer_gettimeofday: timer.cc timer.h
#	$(CC) $(MOREFLAGS) $(USER_OPT) $(CFLAGS) -o $@ $< $(LIB) -DSTAPL_SELF_TEST -DSTAPL_TIMER_GETTIMEOFDAY
#timer_getrusage: timer.cc timer.h
#	$(CC) $(MOREFLAGS) $(USER_OPT) $(CFLAGS) -o $@ $< $(LIB) -DSTAPL_SELF_TEST -DSTAPL_TIMER_GETRUSAGE
#timer_papi: timer.cc timer.h
#	$(CC) $(MOREFLAGS) $(USER_OPT) $(CFLAGS) -o $@ $< $(LIB) -DSTAPL_SELF_TEST -DSTAPL_TIMER_PAPI

clean: rts_clean
	rm -rf *.o core* a.out timer_clock_gettime timer_gettimeofday timer_getrusage timer_papi

# EOF
