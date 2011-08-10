VIZMO_DIR=..
include $(VIZMO_DIR)/M_incl

ifeq ($(RPM), 0)
TARGET=$(VIZMO_DIR)/lib/libgli.a
else
TARGET=$(VIZMO_DIR)/lib/libgli.so
endif

OBJECTS = gli.o gliCamera.o gliDataStructure.o gliPickBox.o gliTransTool.o gliFont.o gliDump.o

ifeq ($(RPM), 0)
$(TARGET): $(OBJECTS)
	ar ruv $@ $(OBJECTS)
#	libtool -static -o $@ $(OBJECTS)
else
$(TARGET): $(OBJECTS)
	$(CC) -shared $(OBJECTS) -Wl,-soname=`basename $@`.0 -o $@
endif

default_target: $(TARGET)

CLEAN= ${TARGET} ${OBJECTS} Dependencies

-include Dependencies
