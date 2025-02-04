TEMPLATE = app
TARGET = vizmo++
CONFIG += qt gui 


# Directory Layout variables
VIZMO_DIR = .
PMPL_DIR  = $${VIZMO_DIR}/../ppl/src
UTILS_DIR = $${VIZMO_DIR}/../ppl/pmpl_utils
STAPL_DIR = $${UTILS_DIR}/stapl_release


SOURCES += $$files(src/**/*.cpp)
HEADERS += $$files(src/**/*.h)


# includes
GL_INCL = /usr/include/GL


INCLUDES += $${VIZMO_DIR} $${GL_INCL} $${MP_INCL}


