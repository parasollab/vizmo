VIZMO_DIR = ..
include $(VIZMO_DIR)/M_incl

Env_DIR = EnvObj
Map_DIR = MapObj
GUI_DIR = gui

#--------------------------------------------------------------------

target = ../vizmo++

#--------------------------------------------------------------------
OBJS =  main.o vizmo2.o

EnvOBJ = ${Env_DIR}/BoundingBoxModel.o  ${Env_DIR}/BoundingBoxParser.o \
	 ${Env_DIR}/BoundingBoxesModel.o ${Env_DIR}/PathLoader.o  \
         ${Env_DIR}/PathModel.o  ${Env_DIR}/QueryLoader.o \
         ${Env_DIR}/QueryModel.o  ${Env_DIR}/Robot.o \
         ${Env_DIR}/DebugLoader.o  ${Env_DIR}/DebugModel.o

GUIOBJ = -lvgui

#--------------------------------------------------------------------

ifeq ($RPM), 0)
$(target):  ${EnvOBJ} ${OBJS}
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) ${EnvOBJ} ${GUIOBJ} ${LIB}
else
$(target):  ${EnvOBJ} ${OBJS}
	$(CXX) $(CXXFLAGS) -Wl,-rpath=/usr/lib/vizmo++ -o $@ $(OBJS) ${EnvOBJ} ${GUIOBJ} ${LIB}
endif

default_target: $(target)

CLEAN = ${EnvOBJ} ${OBJS} $(target) Dependencies

-include  Dependencies

