#!/bin/sh

run_command=$1
echo $run_command

eval $run_command ./simplepointerjumping_tester 10 Tester_SimplePointerJumping b 4
if test $? != 0
then
    echo "ERROR:: while testing simplepointerjumping_tester"
    exit
fi

eval $run_command ./prec_tester 10 Tester_Recurrence1 b 4
if test $? != 0
then
    echo "ERROR:: while testing prec_tester"
    exit
fi

# mpirun -np 4 sort_tester 1(agg) Tester_Sort b 64(ne) 0(type) 1(presort) 1(sort_used)
# sort_used: 0-adaptive, 1-sample, 2-column, 3-bitonic, 4-radix
# presort: 0-inorder, 1-2% swapped, 2-random, 3-reverse
# type: 0-integers, 1-doubles, 2-points 

eval $run_command ./sort_tester 10 Tester_Sort b 64 0 1 4
if test $? != 0
then
    echo "ERROR:: while testing radix sort with nearly sorted integers"
    exit
fi

eval $run_command ./sort_tester 10 Tester_Sort b 64 2 2 1
if test $? != 0
then
    echo "ERROR:: while testing sample sort with random points"
    exit
fi

eval $run_command ./sort_tester 10 Tester_Sort b 64 1 2 2
if test $? != 0
then
    echo "ERROR:: while testing column sort with random doubles"
    exit
fi

eval $run_command ./sort_tester 10 Tester_Sort b 64 1 3 3
if test $? != 0
then
    echo "ERROR:: while testing bitonic sort with reverse doubles"
    exit
fi

eval $run_command ./inner_product_tester 10 Tester_InnerProduct b 64 0 2
if test $? != 0
then
    echo "ERROR:: while testing inner_product with integers"
    exit
fi

