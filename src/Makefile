include Makefile.VizmoDefaults

INCPATH = -I.

OBJECTS =  main.o vizmo2.o

EXTERNALLIBS = $(RAPID_LIBFILE) $(MATHTOOL_LIBFILE) $(MODELLOADER_LIBFILE)
INTERNALLIBS = $(GLI_LIBFILE) $(MODELGRAPH_LIBFILE) \
	       $(PLUM_LIBFILE) $(CD_LIBFILE) \
	       $(VGUI_LIBFILE) $(ENVOBJ_LIBFILE)

default_target: $(EXTERNALLIBS) $(INTERNALLIBS) $(OBJECTS) vizmo

vizmo: $(EXTERNALLIBS) $(INTERNALLIBS) $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) ${LIB}

reallyclean:
	@${MAKE} clean
	cd GL; ${MAKE} clean
	cd modelgraph; ${MAKE} clean
	cd Plum; ${MAKE} clean
	cd CollisionDetection; ${MAKE} clean
	cd gui; ${MAKE} clean
	cd EnvObj; ${MAKE} clean
	rm -f lib/*.a

reallyreallyclean:
	@${MAKE} reallyclean
	cd $(RAPID_DIR); ${MAKE} clean
	cd $(MATHTOOL_DIR); ${MAKE} clean
	cd $(MODELLOADER_DIR); ${MAKE} clean
