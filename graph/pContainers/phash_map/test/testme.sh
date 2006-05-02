#!/bin/sh

run_command=$1
echo $run_command

eval $run_command ./test_phash
if test $? != 0
then
    echo "ERROR:: while testing test_phash_map"
    exit
fi

#if here everything is fine so we print PASS
echo PASS
