#!/bin/sh

run_command=$1
echo $run_command 

eval $run_command ./test_pmatrix 50
if test $? != 0
then
    echo "ERROR:: while testing test_matrix"
    exit
fi

eval $run_command ./test_pmatrixmult 20
if test $? != 0
then
    echo "ERROR:: while testing test_matrixmult"
    exit
fi

