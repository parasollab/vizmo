VIZMO_DIR=..
include $(VIZMO_DIR)/M_incl

ifeq ($(RPM), 0)
TARGET = ../lib/libmodelgraph.a
else
TARGET = ../lib/libmodelgraph.so
endif

OBJS = ModelGraph.o ModelNode.o ModelEdge.o

ifeq ($(RPM), 0)
$(TARGET): ${OBJS}
	ar ruv $@ $(OBJS)
#	libtool -static -o $@ $(OBJS)
else
$(TARGET): ${OBJS}
	$(CC) -shared $(OBJS) -Wl,-soname=`basename $@`.0 -o $@
endif

default_target: $(TARGET)

CLEAN = $(OBJS) $(TARGET) Dependencies

-include Dependencies
