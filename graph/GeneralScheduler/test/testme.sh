#!/bin/sh

run_command=$1
eval $run_command ./GeneralScheduler_test 5 1 INPUT.DAG 1 >  CPM.test1

awk -F"0:" '{print $NF }' CPM.test1 > CPM.test
diff CPM.test CPM.out
if test $? != 0
then
    echo "(GENERAL_SCHEDULER) test CPM - FAILED"
else
    echo "(GENERAL_SCHEDULER) test CPM - PASSED"
fi

eval $run_command ./GeneralScheduler_test 5 1 INPUT.DAG 2 >  DSC.test1

awk -F"0:" '{print $NF }' DSC.test1 > DSC.test
diff DSC.test DSC.out
if test $? != 0
then
    echo "(GENERAL_SCHEDULER) test DSC - FAILED"
else
    echo "(GENERAL_SCHEDULER) test DSC - PASSED"
fi

eval $run_command ./GeneralScheduler_test 5 1 INPUT.DAG 3 >  FLB.test1

awk -F"0:" '{print $NF }' FLB.test1 > FLB.test
diff FLB.test FLB.out
if test $? != 0
then
    echo "(GENERAL_SCHEDULER) test FLB - FAILED"
else
    echo "(GENERAL_SCHEDULER) test FLB - PASSED"
fi

eval $run_command ./GeneralScheduler_test 5 1 INPUT.DAG 4 >  FCP.test1

awk -F"0:" '{print $NF }' FCP.test1 > FCP.test
diff FCP.test FCP.out
if test $? != 0
then
    echo "(GENERAL_SCHEDULER) test FCP - FAILED"
else
    echo "(GENERAL_SCHEDULER) test FCP - PASSED"
fi

rm -rf CPM.test1 DSC.test1 FLB.test1 FCP.test1 CPM.test DSC.test FLB.test FCP.test
