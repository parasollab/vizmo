mpirun -np 1 ./seulertour_tester Tester_sET p 10000 0  
mpirun -np 1 ./peulertour_tester 10 Tester_pET p 10000 0
mpirun -np 2 ./peulertour_tester 10 Tester_pET p 10000 0
mpirun -np 4 ./peulertour_tester 10 Tester_pET p 10000 0
