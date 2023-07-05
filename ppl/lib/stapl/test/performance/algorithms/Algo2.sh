#
NP=0
case "$1"
in
2) NP=$1 ;;
4) NP=$1 ;;
8) NP=$1 ;;
16) NP=$1 ;;
esac
if [ "$NP" -eq 0 ]
then
  echo "Invalid or missing parallelism specification: " $1
  exit
fi

EXEC='/usr/lib64/mpich/bin/mpiexec'
export STAPL_NUM_THREADS=1
MODEL=medium
OK=1
date

if [ "$OK" -eq 1 ]
then
$EXEC -n $NP ./ispart.exe     -data $MODEL 
$EXEC -n $NP ./equal.exe      -data $MODEL 
$EXEC -n $NP ./maxelement.exe -data $MODEL 
$EXEC -n $NP ./maxvalue.exe   -data $MODEL 
$EXEC -n $NP ./minelement.exe -data $MODEL 
$EXEC -n $NP ./minvalue.exe   -data $MODEL 
$EXEC -n $NP ./allanynone.exe -data $MODEL 
$EXEC -n $NP ./isperm.exe     -data $MODEL
else
$EXEC -n $NP ./count.exe      -data $MODEL # extremely slow
fi

##gnuplot -e "load algo2_plots"
