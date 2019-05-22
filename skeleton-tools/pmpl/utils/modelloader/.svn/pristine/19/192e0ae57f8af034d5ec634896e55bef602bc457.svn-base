#makefile for modelloader library
include ../makefile_includes/Makefile.defaults

#--------------------------------------------------------------------
INCL = -I. -I/usr/local/include -I../mathtool/
ifeq (${platform}, CRAY_gcc)
  INCL += -I${BOOST_ROOT}/include -I/mnt/lustre/lus0/fourcow2/CGAL/include/
endif

OBJS = IModel.o ModelFactory.o MovieBYULoader.o ObjLoader.o BVHDataLoader.o
TARGET = libmodloader.a

all: $(TARGET)
#--------------------------------------------------------------------
default_target: library

library: $(TARGET)

$(TARGET): $(OBJS)
	ar rUuv $@  $(OBJS)

CLEAN = $(TARGET) $(OBJECTS) *.a *.o *~ *.so
