mpirun -np 1 sgraph_tester -testall 10 2 
mpirun -np 1 pgraph_tester 10 -testall 10 2
mpirun -np 2 pgraph_tester 10 -testall 10 2
mpirun -np 4 pgraph_tester 10 -testall 10 2
