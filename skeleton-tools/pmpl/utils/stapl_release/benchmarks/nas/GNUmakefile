# Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
# component of the Texas A&M University System.
#
# All rights reserved.
#
# The information and source code contained herein is the exclusive
# property of TEES and may not be disclosed, examined or reproduced
# in whole or in part without explicit written authorization from TEES.

ifndef STAPL
  STAPL = $(shell echo "$(PWD)" | sed 's,/benchmarks/nas,,')
endif

include $(STAPL)/GNUmakefile.STAPLdefaults

default: compile

test: all
	./testme.sh "$(call staplrun,1)"
	./testme.sh "$(call staplrun,2)"
	./testme.sh "$(call staplrun,4)"

all: compile

compile: is is_skeletons ep ft

clean:
	rm -rf *.o core* a.out cg ep is is_skeletons ft makea.o randi8.o

veryclean:
	rm -rf *.o core* a.out cg *v3breakpoints*

makea.o: makea.f
	gfortran -c -O3 -m64 makea.f

randi8.o: randi8.f
	gfortran -c -O3 -m64 randi8.f

cg: cg.cc makea.o randi8.o
	${CC} ${STAPL_CXXFLAGS} ${CXXFLAGS} -o cg cg.cc makea.o randi8.o ${STAPL_LIBRARIES} -lgfortran ${LIB} ${LIB_EPILOGUE}

ep: ep.cc randi8.o
	${CC} ${STAPL_CXXFLAGS} ${CXXFLAGS} -o ep ep.cc randi8.o ${STAPL_LIBRARIES} -lgfortran ${LIB} ${LIB_EPILOGUE}
