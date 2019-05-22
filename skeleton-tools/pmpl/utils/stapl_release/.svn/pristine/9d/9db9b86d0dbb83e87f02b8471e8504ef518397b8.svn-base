#!/bin/sh

# Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
# component of the Texas A&M University System.
#
# All rights reserved.
#
# The information and source code contained herein is the exclusive
# property of TEES and may not be disclosed, examined or reproduced
# in whole or in part without explicit written authorization from TEES.

run_command=$1

##NOTE: Uncomment these lines as and when ported, Delete this note when all have been ported.

eval $run_command ./test_profiler 1000 --ntimes 10 --pcntimes 4 --useconfrange > profile_log
eval $run_command ./profiler_p_array 0 10000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
eval $run_command ./profiler_p_array 1 10000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
eval $run_command ./profiler_p_array 2 10000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
eval $run_command ./profiler_p_array 3 10000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
eval $run_command ./profiler_p_array 4 10000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
eval $run_command ./profiler_p_array 5 10000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
eval $run_command ./profiler_p_graph 1 1000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
eval $run_command ./profiler_p_graph 0 1000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
#eval $run_command ./profiler_assoc 0 1000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
#eval $run_command ./profiler_assoc 1 1000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
#eval $run_command ./profiler_assoc 2 1000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
#eval $run_command ./profiler_assoc 3 1000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
#eval $run_command ./profiler_assoc 4 1000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
#eval $run_command ./profiler_assoc 5 1000 --ntimes 10 --pcntimes 4 --useconfrange >> profile_log
#eval $run_command ./profiler_p_container_composition 100 100 --useconfrange >> profile_log
#eval $run_command ./profiler_p_array_algo  --useconfrange >> profile_log
#eval $run_command ./profiler_p_list_algo  --useconfrange >> profile_log
#eval $run_command ./profiler_p_vector_algo  --useconfrange >> profile_log
#eval $run_command ./profiler_p_multiarray_array 1000  --useconfrange >> profile_log

if test $? != 0
then
    echo "ERROR:: while testing test_profiler"
fi
