# Directory Layout ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

RAPID_DIR := RAPID
PQP_DIR   := PQP_v1.2

# Libraries ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

# Default target is to build all CD libraries.
.DEFAULT_GOAL = all
all: rapid pqp

rapid:
	@echo "Building RAPID library..."
	@cd $(RAPID_DIR) && $(MAKE)

pqp:
	@echo "Building PQP library..."
	@cd $(PQP_DIR) && $(MAKE)

# Cleanup ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

.PHONY: clean
clean: clean_rapid clean_pqp

.PHONY: clean_rapid
clean_rapid:
	@cd $(RAPID_DIR) && $(MAKE) clean

.PHONY: clean_pqp
clean_pqp:
	@cd $(PQP_DIR) && $(MAKE) clean

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
