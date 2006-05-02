#!/bin/sh

run_command=$1
echo $run_command

eval $run_command ./test_parray 100
if test $? != 0
then
    echo "ERROR:: while testing test_parray"
    exit
fi
