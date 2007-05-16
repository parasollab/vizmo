
ifndef STAPL
  STAPL = ../../../stapl
endif
include $(STAPL)/GNUmakefile.STAPLdefaults

default: stapl_perf_db_v1 stapl_perf_db

stapl_perf_db_v1:  stapl_perf_db_v1.h  stapl_perf_db_v1.cc
	$(CC) $(MOREFLAGS) $(USER_OPT) $(CFLAGS) $(LIB) -o $@ $@.cc

stapl_perf_db: stapl_perf_db_v1.h stapl_perf_db.h stapl_perf_db.cc
	$(CC) $(MOREFLAGS) $(USER_OPT) $(CFLAGS) $(LIB) -o $@ $@.cc

clean:
	rm -f stapl_perf_db stapl_perf_db_v1

# EOF
