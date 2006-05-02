#!/bin/sh
no=10
while test 1 -eq 1
do
    echo "testing size " $no 
    ./gen $no 1 $no > out
    cp out in
    echo 1
    ./tstn1 > a1
    echo 2
    ./tsto1 > a2
    echo diff
    diff a1 a2
	
   echo 1
    ./tstn2 > a1
    echo 2
    ./tsto2 > a2
    echo diff
    diff a1 a2

    echo 1
    ./tstn3 > a1
    echo 2
    ./tsto3 > a2
    echo diff
    diff a1 a2

   echo 1
    ./tstn4 > a1
    echo 2
    ./tsto4 > a2
    echo diff
    diff a1 a2
    no=`expr $no + 5`
done

