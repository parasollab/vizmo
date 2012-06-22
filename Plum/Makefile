VIZMO_DIR=..
include $(VIZMO_DIR)/M_incl

Env_DIR = EnvObj
Map_DIR = MapObj

ifeq ($(RPM), 0)
TARGET=../lib/libPlum.a
else
TARGET=../lib/libPlum.so
endif

OBJECTS = Plum.o PlumState.o PlumUtil.o

EnvOBJ = ${Env_DIR}/CmdParser.o  ${Env_DIR}/EnvLoader.o  ${Env_DIR}/EnvModel.o  \
         ${Env_DIR}/MovieBYULoader.o  ${Env_DIR}/PolyhedronModel.o \
         ${Env_DIR}/MultiBodyInfo.o  ${Env_DIR}/MultiBodyModel.o \
	 ${Env_DIR}/Transformation.o  ${Env_DIR}/Orientation.o \
	 ${Env_DIR}/RobotInfo.o
MapOBJ = ${Map_DIR}/MapLoader.o  ${Map_DIR}/MapModel.o  ${Map_DIR}/Cfg.o ${Map_DIR}/Edge.o 

ifeq ($(RPM), 0)
$(TARGET): ${OBJECTS} ${EnvOBJ} ${MapOBJ}
	ar ruv $@  ${OBJECTS} ${EnvOBJ} ${MapOBJ}
#	libtool -static -o $@  ${OBJECTS} ${EnvOBJ} ${MapOBJ}
else
$(TARGET): ${OBJECTS} ${EnvOBJ} ${MapOBJ}
	$(CC) -shared ${OBJECTS} ${EnvOBJ} ${MapOBJ} -Wl,-soname=`basename $@`.0 -o $@
endif

default_target: $(TARGET)

CLEAN = ${TARGET} ${OBJECTS} ${EnvOBJ} ${MapOBJ} Dependencies

-include Dependencies


