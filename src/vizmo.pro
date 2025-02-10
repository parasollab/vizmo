
# include other .defaults makefiles. 
VIZMO_DIR = .
PMPL_DIR  = $${VIZMO_DIR}/../ppl/src
UTILS_DIR = $${VIZMO_DIR}/../ppl/pmpl_utils
STAPL_DIR = $${UTILS_DIR}/stapl_release

# import the PPL lib variable NAMES
include($${PMPL_DIR}/vizmoincludes.pri)

TEMPLATE = app
TARGET = vizmo++
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += $${CXXFLAGS}

QT += gui widgets 
SOURCES += main.cpp



# includes from pmpl UTILS
INCLUDEPATH += $${MATHTOOL_DIR}
INCLUDEPATH += $${PMPL_DIR}
