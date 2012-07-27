VIZMO_DIR=..

Env_DIR = EnvObj
Map_DIR = MapObj

TARGET=../lib/libPlum.a

OBJS = Plum.o PlumState.o PlumUtil.o

ENVOBJS = ${Env_DIR}/CmdParser.o ${Env_DIR}/EnvLoader.o  ${Env_DIR}/EnvModel.o  \
         ${Env_DIR}/MovieBYULoader.o  ${Env_DIR}/PolyhedronModel.o \
         ${Env_DIR}/MultiBodyInfo.o  ${Env_DIR}/MultiBodyModel.o \
	 ${Env_DIR}/Transformation.o  ${Env_DIR}/Orientation.o \
	 ${Env_DIR}/RobotInfo.o

MAPOBJS = ${Map_DIR}/MapLoader.o  ${Map_DIR}/MapModel.o  ${Map_DIR}/Cfg.o ${Map_DIR}/Edge.o 

OBJECTS = $(OBJS) $(ENVOBJS) $(MAPOBJS) 

include $(VIZMO_DIR)/Makefile.VizmoDefaults
