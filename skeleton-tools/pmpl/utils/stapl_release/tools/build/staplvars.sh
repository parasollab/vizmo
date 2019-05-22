#!/bin/bash
# Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
# component of the Texas A&M University System.
#
# All rights reserved.
#
# The information and source code contained herein is the exclusive
# property of TEES and may not be disclosed, examined or reproduced
# in whole or in part without explicit written authorization from TEES.

# error printing
error()
{
  local parent_lineno="$1"
  local message="$2"
  local code="${3:-1}"
  if [[ -n "$message" ]] ; then
    echo -e "Line ${parent_lineno}: ${message}; exiting with status ${code}"
  else
    echo -e "Line ${parent_lineno}; exiting with status ${code}"
  fi
  exit "${code}"
}

# check if debug was added
if [[ "x$1" != "x" ]]; then
  if [[ "$1" != "debug" ]]; then
    error ${LINENO} "Only valid option is \"debug\"" 1
  fi
  debug=1
fi

# find the absolute parent directory the script is in
script_path="$( realpath ${BASH_SOURCE[0]} )"

# STAPL root path
STAPL_ROOT="$( cd -P "$( dirname "${script_path}" )" && cd .. && pwd)"
export STAPL_ROOT

# find which libstd we are using
if [[ "x${STAPL_STL_DIR}" = "x" ]] ; then
  # check if gcc version was given
  if [[ "x${GCC_VERSION}" = "x" ]] ; then
    GCC_VERSION=`gcc -dumpversion`
    # In some gcc installation, dumpversion is not working correctly and gives
    # only major and minor version. Use this line to attempt to fix that.
    #GCC_VERSION=`gcc --version | grep ^gcc | sed 's/^.* //g'`
    if [[ "x${GCC_VERSION}" = "x" ]] ; then
      error ${LINENO} "Could not automatically find gcc version. Either define one with GCC_VERSION or set STAPL_STL_DIR to the directory in tools/ that contains the headers for the compiler that will be used to compile STAPL applications." 1
    fi
    STAPL_STL_DIR="${STAPL_ROOT}/tools/libstdc++/${GCC_VERSION}"
  fi
fi

# check if the required STL directory exists
if [ ! -d ${STAPL_STL_DIR} ] ; then
  error ${LINENO} "${STAPL_STL_DIR} could not be found" 1
fi

# filled out by external methods (makefile, script etc)
ADDITIONAL_FLAGS="%ADDITIONAL_FLAGS%"
ADDITIONAL_LIBS="%ADDITIONAL_LIBS%"
LIB_SUFFIX="%LIB_SUFFIX%"

# STAPL required flags and libs
if [[ "x$debug" = "x" ]]; then
  # release
  STAPL_FLAGS="-D_STAPL -DSTAPL_NDEBUG -I${STAPL_STL_DIR} -I${STAPL_ROOT}/tools -I${STAPL_ROOT} ${ADDITIONAL_FLAGS}"
  STAPL_LIBS="-L${STAPL_ROOT}/lib -lstapl${LIB_SUFFIX} ${ADDITIONAL_LIBS}"
else
  # debug
  STAPL_FLAGS="-D_STAPL -I${STAPL_STL_DIR} -I${STAPL_ROOT}/tools -I${STAPL_ROOT} ${ADDITIONAL_FLAGS}"
  STAPL_LIBS="-L${STAPL_ROOT}/lib -lstapl${LIB_SUFFIX}_debug  ${ADDITIONAL_LIBS}"
fi
export STAPL_FLAGS
export STAPL_LIBS

printf "STAPL variables loaded:\n"
printf "  STAPL_FLAGS=${STAPL_FLAGS}\n"
printf "  STAPL_LIBS=${STAPL_LIBS}\n"
printf "\nCompile your STAPL application using:\n"
printf "  \$CC \$STAPL_FLAGS \$CXX_FLAGS app.cc -o app \$LIBS \$STAPL_LIBS\n"
