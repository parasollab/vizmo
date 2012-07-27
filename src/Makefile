INCPATH = -I.
VIZMO_DIR = ..

Env_DIR = EnvObj
Map_DIR = MapObj
GUI_DIR = gui

TARGET = ../vizmo++

OBJS =  main.o vizmo2.o

ENVOBJS = ${Env_DIR}/BoundingBoxModel.o  ${Env_DIR}/BoundingBoxParser.o \
	 ${Env_DIR}/BoundingBoxesModel.o ${Env_DIR}/PathLoader.o  \
         ${Env_DIR}/PathModel.o  ${Env_DIR}/QueryLoader.o \
         ${Env_DIR}/QueryModel.o  ${Env_DIR}/Robot.o \
         ${Env_DIR}/DebugLoader.o  ${Env_DIR}/DebugModel.o

OBJECTS = $(OBJS) $(ENVOBJS)

include $(VIZMO_DIR)/Makefile.VizmoDefaults

$(TARGET):  $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) ${LIB}
