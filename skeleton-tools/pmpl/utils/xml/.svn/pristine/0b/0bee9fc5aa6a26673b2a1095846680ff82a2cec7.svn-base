include ../../makefile_includes/Makefile.defaults

# TINYXML_USE_STL can be used to turn on STL support. NO, then STL
# will not be used. YES will include the STL files.
TINYXML_USE_STL = YES
ifeq (YES, ${TINYXML_USE_STL})
  DEFS = -DTIXML_USE_STL
else
  DEFS =
endif

SRCS = tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp tinystr.cpp
OBJS = $(addsuffix .o,$(basename ${SRCS}))

CLEAN = core ${OBJS} libtinyxml.a

# default target
default_target: libs

libs: libtinyxml.a

libtinyxml.a: ${OBJS}
	${AR} libtinyxml.a ${OBJS}

tinyxml.o: tinyxml.h tinystr.h
tinyxmlparser.o: tinyxml.h tinystr.h
xml-pmp.o: tinyxml.h tinystr.h xml-pmp.h
tinyxmlerror.o: tinyxml.h tinystr.h
