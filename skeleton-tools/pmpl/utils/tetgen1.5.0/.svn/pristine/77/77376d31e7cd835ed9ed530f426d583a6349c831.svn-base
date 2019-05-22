include ../makefile_includes/Makefile.defaults

FLAGS := -std=c++11 -O2
DEFS  := -DTETLIBRARY

OBJECTS	= predicates.o tetgen.o

CLEAN = ${OBJECTS} *.a *.o *~

# Recipies

default_target: library

library: libtet.a

libtet.a: ${OBJECTS}
	${AR} libtet.a ${OBJECTS}

%.o: %.cxx
	@${MAKE} -s check_platform
	${CXX} -c ${FLAGS} ${DEFS} $< -o $@
