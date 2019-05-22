# Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
# component of the Texas A&M University System.
#
# All rights reserved.
#
# The information and source code contained herein is the exclusive
# property of TEES and may not be disclosed, examined or reproduced
# in whole or in part without explicit written authorization from TEES.

ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/test/algorithms/unit,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

OBJS:= adjacent_difference adjacent_find binary_search dynamic_programming \
       find_end find_first_of generate lexicographic_compare merge \
       min_element mismatch partition_copy \
       random_shuffle rotate_copy search search_n \
       test_is_partitioned test_map_reduce test_n_partition \
       test_partition_point minmax test_summa

# not compiling: fill next_prev_permutation test_euler_tour test_inner_view

.PHONY:compile test clean
default: compile

compile: $(OBJS)

test: compile
	./testme.sh "$(call staplrun,1)"
	./testme.sh "$(call staplrun,2)"
	./testme.sh "$(call staplrun,3)"
	./testme.sh "$(call staplrun,4)"

clean:
	rm -rf *.o *.i *.err *~ *.TVD.* *.o core* a.out ii_files rii_files results.* $(OBJS)
