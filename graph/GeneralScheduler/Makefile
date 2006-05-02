STAPL = ../..
INCLUDE  = -I. -I$(STAPL)/include/stlport -I$(STAPL)/include

ifndef platform
  $(warning USAGE: gmake platform=P rts=R [target],)
  $(warning where P={HP_aCC, IBM_g++, IBM_xlC, LINUX_g++, LINUX_g++_noSTLport, LINUX_KCC, SGI_CC, BLRTS_g++, BLRTS_g++_noSTLport})
  $(warning and   R={MPI, THREAD, MIXED})
  $(error ***)
endif

ifeq ($(platform), SGI_CC)
CFLAGS = $(INCLUDE) -D_STL_PORT -D_STAPL_MPI -D_STAPL
CC     = CC -g -O0 -LANG:libc_in_namespace_std=OFF
endif

ifeq ($(platform), LINUX_g++)
CFLAGS = $(INCLUDE) -D_STL_PORT -D_STAPL_MPI -D_STAPL
CC     = mpiCC -g -O0
endif

ifeq ($(platform), LINUX_g++_noSTLport)
CFLAGS = $(INCLUDE) -D_STAPL_MPI -D_STAPL 
CC     = mpiCC -g -O0
endif

ifeq ($(platform), IBM_g++)
CFLAGS = $(INCLUDE) -D_STL_PORT -D_STAPL_MPI -D_STAPL
CC     = mpg++-3.2.1 -g -O0
endif 

ifeq ($(platform), HP_aCC)
CFLAGS = -AA -D_POSIX_C_SOURCE=199506L -D_HPUX_SOURCE $(INCLUDE) -D_STL_PORT -D_STAPL_MPI -D_STAPL
CC     = mpiCC -g -O0
endif

Schedule.o: Schedule.cc
	$(CC) $(CFLAGS) -c Schedule.cc 

clean:
	rm -f Schedule.o
