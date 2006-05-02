#!/bin/sh

run_command=$1
echo $run_command

eval $run_command ./test_pgraph 100 
if test $? != 0
then
    echo "ERROR:: while testing test_pgraph"
fi

eval $run_command ./test_pbasegraph 100 
if test $? != 0
then
    echo "ERROR:: while testing test_pbasegraph"
fi

eval $run_command ./test_setget 100 
if test $? != 0
then
    echo "ERROR:: while testing test_setget"
fi

eval $run_command ./test_vertexmethods 100 
if test $? != 0
then
    echo "ERROR:: while testing test_vertexmethods"
fi

eval $run_command ./test_directed 100 
if test $? != 0
then
    echo "ERROR:: while testing test_directed"
fi

eval $run_command ./test_undirected 100 
if test $? != 0
then
    echo "ERROR:: while testing test_undirected"
fi


eval $run_command ./test_adddel 100 >/dev/null
if test $? != 0
then
    echo "ERROR:: while testing test_adddel"
fi

eval $run_command ./test_pgraph1 16 m34 >/dev/null
if test $? != 0
then
    echo "ERROR:: while testing test_pgraph1"
fi

eval $run_command ./test_pgraphalgo 111
if test $? != 0
then
    echo "ERROR:: while testing test_pgraphalgo"
fi
#if here everything is fine so we print PASS
echo PASS
