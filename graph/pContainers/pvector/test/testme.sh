#!/bin/sh

run_command=$1
echo $run_command 

eval $run_command ./test_pvector 100

exit
#alin verify bellow and update 

cd convex_hull
make clean
make ch
mpirun -np 2 ParQuickConvexHull 100 100 100 >/dev/null
if test $? != 0
then
    echo "ERROR:: while testing test_pvector"
    exit
fi

#if here everything is fine so we print PASS
echo PASS
