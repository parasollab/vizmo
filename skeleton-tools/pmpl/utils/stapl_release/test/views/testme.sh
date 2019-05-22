#!/bin/bash

# Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
# component of the Texas A&M University System.
#
# All rights reserved.
#
# The information and source code contained herein is the exclusive
# property of TEES and may not be disclosed, examined or reproduced
# in whole or in part without explicit written authorization from TEES.

run_command=$1

reg="[1]"

if [[ $run_command =~ $reg ]]
then
  eval $run_command ./test_stl_view
  if test $? != 0
  then
      echo "ERROR:: while testing test_stl_view"
  fi
fi

eval $run_command ./test_domain_view
if test $? != 0
then
    echo "ERROR:: while testing test_domain_view"
fi

eval $run_command ./test_counting 1234
if test $? != 0
then
    echo "ERROR:: while testing test_counting"
fi

eval $run_command ./test_partitioned_view 1234
if test $? != 0
then
    echo "ERROR:: while testing test_partitioned_view"
fi

eval $run_command ./test_strided_view 1000
if test $? != 0
then
    echo "ERROR:: while testing test_strided_view"
fi

eval $run_command ./test_zip_view 1031
if test $? != 0
then
    echo "ERROR:: while testing test_zip_view"
fi

eval $run_command ./test_pair_proxy 1000
if test $? != 0
then
    echo "ERROR:: while testing test_pair_proxy"
fi

eval $run_command ./test_vector_proxy 100
if test $? != 0
then
    echo "ERROR:: while testing test_vector_proxy"
fi

eval $run_command ./test_set_proxy
if test $? != 0
then
echo "ERROR:: while testing test_set_proxy"
fi

eval $run_command ./test_periodic_boundary_view 10 12 15
if test $? != 0
then
    echo "ERROR:: while testing test_periodic_boundary_view"
fi

eval $run_command ./test_repeat_view
if test $? != 0
then
    echo "ERROR:: while testing test_repeat_view"
fi

eval $run_command ./test_power_method
if test $? != 0
then
    echo "ERROR:: while testing test_power_method"
fi

eval $run_command ./test_slices_view 4
if test $? != 0
then
    echo "ERROR:: while testing slices view"
fi

eval $run_command ./test_composed_slices_algo 5 7
if test $? != 0
then
    echo "ERROR:: while testing composed slices view"
fi

eval $run_command ./test_extended_view 4
if test $? != 0
then
    echo "ERROR:: while testing extended view"
fi

eval $run_command ./test_sliced_extended_view 16
if test $? != 0
then
    echo "ERROR:: while testing sliced extended view"
fi

eval $run_command ./test_composed_slices_view 4
if test $? != 0
then
    echo "ERROR:: while testing composed slices view"
fi

eval $run_command ./test_lazy_insert_view 117
if test $? != 0
then
    echo "ERROR:: while testing lazy insert view"
fi

eval $run_command ./segmented_view_metadata 10 10 10 5 2 2
if test $? != 0
then
    echo "ERROR:: while testing segmented view metadata"
fi

if [ -n "${TBBROOT}" ]
then
    eval $run_command ./test_tbb_range_adaptor 5000
    if test $? != 0
    then
        echo "ERROR:: while testing test_tbb_range_adaptor"
    fi
fi
