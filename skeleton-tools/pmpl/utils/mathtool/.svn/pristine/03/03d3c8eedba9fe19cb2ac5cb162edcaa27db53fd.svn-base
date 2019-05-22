ifndef platform
  platform = LINUX_gcc
endif

include ../makefile_includes/Makefile.defaults

CLEAN = test example *~ 

example: example.cpp $(LIBFILE)
	${CXX} ${CXXFLAGS} ${OPTS} $< -o $@

test: test.cpp $(LIBFILE)
	${CXX} ${CXXFLAGS} ${OPTS} $< -o $@
