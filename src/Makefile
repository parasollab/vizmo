# set default platform
ifndef platform
  platform = LINUX_gcc
endif

# set default debug
ifndef debug
  debug = 0
endif

# set PHANToM compile
ifndef phantom
  phantom = 1
endif

# set directory paths
ifndef VIZMO_DIR
  VIZMO_DIR = .
endif

ifndef UTILS_DIR
  UTILS_DIR = $(VIZMO_DIR)/../pmpl/utils
endif

# include makefile defaults
include ${UTILS_DIR}/makefile_includes/Makefile.defaults
include Makefile.VizmoDefaults

# macros
INCL = $(VIZMO_INCL)
LIBS = $(VIZMO_LIBS)

default_target: $(MP_LIBFILE) $(VIZMO_LIBFILE) vizmo++

$(VIZMO_OBJECTS): ${STAPL_LIBFILE} ${CD_LIBFILE} ${XML_LIBFILE} ${MPNN_LIBFILE} ${MODELLOADER_LIBFILE} ${KMEANS_LIBFILE}

vizmo++: $(MP_LIBFILE) $(VIZMO_OBJECTS) $(VIZMO_LIBFILE) main.cpp
	$(CXX) $(CXXFLAGS) $(OPTS) $(DEFS) $(INCL) main.cpp $(VIZMO_LIBFILE) $(LIBS) -o $@

%.o: %.cpp
	@${MAKE} -s check_platform
	${CXX} -c ${CXXFLAGS} ${OPTS} -MMD ${DEFS} ${INCL} ${DEPS} $< -o $@
	cat $*.d >> Dependencies
	rm -f $*.d

CLEAN= $(VIZMO_LIBFILE) vizmo++

reallyclean:
	@${MAKE} clean
	rm -f $(VIZMO_OBJECTS) $(VIZMO_SRCMOC) Dependencies

reallyreallyclean:
	@${MAKE} reallyclean
	cd $(PMPL_DIR); ${MAKE} reallyreallyclean

-include Dependencies

