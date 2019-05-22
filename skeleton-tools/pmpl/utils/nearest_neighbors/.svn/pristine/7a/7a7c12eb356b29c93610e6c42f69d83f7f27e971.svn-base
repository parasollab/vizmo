################################################################
#
# Makefile for the Nearest Neighbor Library for Motion Planning
#
# This is the non-configurable part of the Makefile.
# See the file 'Makefile.config' for configuration switches.
#
#
################################################################


include ../../../makefile_includes/Makefile.defaults

INCL = -I. -I../include

OBJECTS = ANN.o kd_tree.o kd_util.o kd_split.o kd_search.o \
	nn.o multiann.o

CLEAN = ${OBJECTS} *.a *.o *~ *.so

#default target
default_target: library

library: libMPNN.a

install: library sample_client

libMPNN.a: ${OBJECTS}
	/bin/rm -f libMPNN.a
	${AR} libMPNN.a ${OBJECTS}

.SUFFIXES: .o .C
.C.o:
	${CXX} -c ${CXXFLAGS} ${OPTS} ${DEFS} ${INCL} $< -o $@
