# include other .defaults makefiles. 
VIZMO_DIR = .
PMPL_DIR  = $${VIZMO_DIR}/../ppl/src
UTILS_DIR = $${VIZMO_DIR}/../ppl/pmpl_utils
STAPL_DIR = $${UTILS_DIR}/stapl_release

# import the PPL lib variable NAMES
# ORDER OF THE NEXT FEW LINES MATTERS. DO NOT REARRANGE.
include(config.pri)
include($${PMPL_DIR}/pplincludes.pri)

TEMPLATE = app
TARGET = vizmo++
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += $${CXXFLAGS}
QT += core widgets gui

SOURCES += main.cpp 
# SOURCES += $${PMPL_SRCS}

# defines
DEFINES += $${PMPL_DEF}

# includes from pmpl UTILS
INCLUDEPATH += $${MATHTOOL_INCL}
INCLUDEPATH += $${PMPL_INCL}
INCLUDEPATH += $${STAPL_INCL}

message($${STAPL_INCL})

