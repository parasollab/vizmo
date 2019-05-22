# Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
# component of the Texas A&M University System.
#
# All rights reserved.
#
# The information and source code contained herein is the exclusive
# property of TEES and may not be disclosed, examined or reproduced
# in whole or in part without explicit written authorization from TEES.

ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/test/containers/profile/run,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

#OBJS:= profiler_hash_map profiler_p_array_algo profiler_p_vector_algo \
#		profiler_p_list_algo profiler_p_list profiler_p_container_composition \
#		profiler_assoc profiler_p_multiarray_array

OBJS:=test_profiler profiler_p_array profiler_p_graph profiler_matrix profiler_p_matrix profiler_multiarray profiler_p_multiarray profiler_vector profiler_p_vector profiler_p_static_array

.PHONY:compile test clean
default: compile

compile: $(OBJS)

#Special Cases
#profiler_p_array_algo: profiler_p_container_algo.cc
#	$(CC) -DPARRAY $(CXXFLAGS) -o $@ profiler_p_container_algo.cc $(LIB)
#profiler_p_vector_algo: profiler_p_container_algo.cc
#	$(CC) -DPVECTOR $(CXXFLAGS) -o $@ profiler_p_container_algo.cc $(LIB)
#profiler_p_list_algo: profiler_p_container_algo.cc
#	$(CC) -DPLIST $(CXXFLAGS) -o $@ profiler_p_container_algo.cc $(LIB)
#profiler_assoc: profiler_assoc.cc
#	$(CC) $(CXXFLAGS) -o $@ $@.cc $(LIB)
#profiler_insert_bench_mark: profiler_insert_bench_mark.cc
#	$(CC) -DPARRAY $(CXXFLAGS) -o $@ profiler_insert_bench_mark.cc $(LIB)
#profiler_p_euler: profiler_p_euler.cc
#	$(CC) -DPARRAY $(CXXFLAGS) -o $@ profiler_p_euler.cc $(LIB)
#profiler_hash_map: profiler_hash_map.cc
#	$(CC) $(CXXFLAGS) $(TBB_OPT) -o $@ profiler_hash_map.cc $(LIB)
#profiler_p_container_composition: profiler_p_container_composition.cc
#	$(CC) $(CXXFLAGS) $(TBB_OPT) -o $@ profiler_p_container_composition.cc $(LIB)
#profiler_p_list: profiler_p_list.cc
#	$(CC) $(CXXFLAGS) -o $@ profiler_p_list.cc $(LIB)
#profiler_p_graph: profiler_p_graph.cc
#	$(CC) $(CXXFLAGS) -o $@ profiler_p_graph.cc $(LIB)
#profiler_p_multiarray_array: profiler_p_multiarray_array.cc
#	$(CC) $(CXXFLAGS) -o $@ profiler_p_multiarray_array.cc $(LIB)

test: compile
	./testme.sh "$(call staplrun,4)"
	./testme.sh "$(call staplrun,1)"

clean:
	rm -rf *.o *.i *.err *~ *.TVD.* *.o core* a.out ii_files rii_files profile_log $(OBJS)
