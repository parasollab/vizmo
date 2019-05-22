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

reg="[1,2,4]"

if [[ $run_command =~ $reg ]]
then
  eval $run_command ./test_communication_skeletons 4 10
  if test $? != 0
  then
      echo "ERROR:: while testing test_communication_skeletons"
  fi
fi

eval $run_command ./test_serial
if test $? != 0
then
  echo "ERROR:: while testing test_serial"
fi

eval $run_command ./test_skeletons 256
if test $? != 0
then
    echo "ERROR:: while testing test_skeletons"
fi

eval $run_command ./test_coarse_skeletons 2048
if test $? != 0
then
    echo "ERROR:: while testing test_coarse_skeletons"
fi

eval $run_command ./test_sorting 256
if test $? != 0
then
    echo "ERROR:: while testing test_sorting"
fi

eval $run_command ./test_multiarray 12
if test $? != 0
then
    echo "ERROR:: while testing test_multiarray"
fi

eval $run_command ./test_nested 10 10
if test $? != 0
then
    echo "ERROR:: while testing test_nested"
fi

eval $run_command ./test_notify_map 1000
if test $? != 0
then
    echo "ERROR:: while testing test_notify_map"
fi

eval $run_command ./test_stencil 15
if test $? != 0
then
    echo "ERROR:: while testing test_stencil"
fi

eval $run_command ./test_wavefront 15
if test $? != 0
then
    echo "ERROR:: while testing test_wavefront"
fi

eval $run_command ./test_doall 11
if test $? != 0
then
    echo "ERROR:: while testing test_doall"
fi
