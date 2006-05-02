#!/bin/sh

run_command=$1
eval $run_command ./test_plist 1000 0 1 0
if test $? != 0
then
    echo "ERROR:: while testing test_plist"
fi
#if here everything is fine so we print PASS
echo PASS
