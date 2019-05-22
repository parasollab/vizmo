include ../../makefile_includes/Makefile.defaults

INCL = -I.

OBJECTS = lib/PQP.o lib/BV.o lib/Build.o lib/TriDist.o

CLEAN = ${OBJECTS} lib/libPQP.a include/*.h

#default target
default_target: library

library: lib/libPQP.a

lib/libPQP.a: ${OBJECTS}
	${AR} lib/libPQP.a ${OBJECTS}
	cp src/*.h include/.

lib/%.o: src/%.cpp
	@${MAKE} -s check_platform
	${CXX} -c ${CXXFLAGS} ${OPTS} ${DEFS} ${INCL} $< -o $@
