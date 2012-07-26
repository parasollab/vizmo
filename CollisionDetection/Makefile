VIZMO_DIR=..
include $(VIZMO_DIR)/M_incl

ifeq ($(RPM), 0)
TARGET=$(VIZMO_DIR)/lib/libCD.a
else
TARGET=$(VIZMO_DIR)/lib/libCD.so
endif

OBJECTS = CollisionDetection.o

ifeq ($(RPM), 0)
$(TARGET): $(OBJECTS)
	$(AR) $@ $(OBJECTS)
else
$(TARGET): $(OBJECTS)
	$(CC) -shared $(OBJECTS) -Wl,-soname=`basename $@`.0 -o $@
endif

default_target: $(TARGET)

CLEAN = ${TARGET} ${OBJECTS} Dependencies

-include Dependencies
