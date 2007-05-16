#!/bin/sh

args=$*
system=`uname -s`
if  [ $system = "Linux" ]
then
make -f Makefile.linux $args
fi
if  [ $system = "IRIX" ]
then
make -f Makefile.sgi $args
fi
if  [ $system = "Darwin" ]
then
make -f Makefile.darwin $args
fi
