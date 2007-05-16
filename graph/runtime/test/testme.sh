#!/bin/sh

run_command=$1
eval $run_command ./testArgTransfer
if test $? != 0
then
    echo "(ARMI) testArgTransfer - FAILED"
else
    echo "(ARMI) testArgTransfer - PASSED"
fi

eval $run_command ./testCommPatterns
if test $? != 0
then
    echo "(ARMI) testCommPatterns - FAILED"
else
    echo "(ARMI) testCommPatterns - PASSED"
fi

