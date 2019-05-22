include ../../makefile_includes/Makefile.defaults

INCL = -I. -I..

OBJECTS	= RAPID.o build.o collide.o overlap.o

CLEAN = sample_client ${OBJECTS} *.a *.o *~ *.so

#default target
default_target: library

library: libRAPID.a

install: library sample_client

libRAPID.a: ${OBJECTS}
	/bin/rm -f libRAPID.a
	${AR} libRAPID.a ${OBJECTS}

.SUFFIXES: .o .C
.C.o:
	@${MAKE} -s check_platform
	${CXX} -c ${CXXFLAGS} ${OPTS} ${DEFS} ${INCL} $< -o $@

sample_client: sample_client.C
	${CXX} ${CXXFLAGS} ${OPTS} ${DEFS} ${INCL} $< -o $@ -L. -lRAPID -lm

