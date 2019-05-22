all: rapid pqp

rapid:
	@echo "compiling rapid..."
	@cd RAPID; ${MAKE}
	@echo "rapid done"

pqp:
	@echo "compiling pqp..."
	@cd PQP_v1.2; ${MAKE}
	@echo "pqp done"


clean: clean_rapid clean_pqp

clean_rapid:
	@echo "cleaning rapid..."
	@cd RAPID; ${MAKE} clean
	@echo "rapid done"

clean_pqp:
	@echo "cleaning pqp..."
	@cd PQP_v1.2; ${MAKE} clean
	@echo "pqp done"
