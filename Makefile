include M_incl

default_target: $(RAPID_LIBFILE) $(MATHTOOL_LIBFILE) $(MODELLOADER_LIBFILE) vizmo

vizmo:
	cd GL; ${MAKE}
	cd modelgraph; ${MAKE}
	cd Plum; ${MAKE}
	cd CollisionDetection; ${MAKE}
	cd src/gui; ${MAKE}
	cd src; ${MAKE}

CLEAN = vizmo++

reallyclean:
	@${MAKE} clean
	cd GL; ${MAKE} clean
	cd modelgraph; ${MAKE} clean
	cd Plum; ${MAKE} clean
	cd CollisionDetection; ${MAKE} clean
	cd src/gui; ${MAKE} clean
	cd src; ${MAKE} clean
	rm -f lib/*.a lib/*.so

reallyreallyclean:
	@${MAKE} reallyclean
	cd $(RAPID_DIR); ${MAKE} clean
	cd $(MATHTOOL_DIR); ${MAKE} clean
	cd $(MODELLOADER_DIR); ${MAKE} clean
