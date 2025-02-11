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
MOC_DIR = $${VIZMO_DIR}/moc
OBJECTS_DIR = $${VIZMO_DIR}/objs
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += $${CXXFLAGS}
QT += core widgets gui opengl openglwidgets

SOURCES += main.cpp 
# SOURCES += $${PMPL_SRCS}

# defines
DEFINES += $${PMPL_DEF}
DEFINES += $${UTILS_DEF}

# includes from pmpl UTILS
# INCLUDEPATH += $${MATHTOOL_INCL}
INCLUDEPATH += $${UTILS_INCL}
# INCLUDEPATH += $${UTILS_LIBFILE}
INCLUDEPATH += $${PMPL_INCL} 
INCLUDEPATH += $${PMPL_LIBFILE}
# INCLUDEPATH += $${STAPL_INCL}
# INCLUDEPATH += $${TINYXML_INCL}
# INCLUDEPATH += $${CGAL_INC}
# INCLUDEPATH += $${CD_INCL}
# INCLUDEPATH += $${TETGEN_INCL}
# INCLUDEPAHT += $${BOOST_INC}


LIBS += $${PMPL_LIB}
LIBS += $${UTILS_LIBS}
LIBS += $${GMP_LIBS} # This MUST be the last libs add. order matters. 

SOURCES +=  $${VIZMO_DIR}/GUI/*.cpp $${VIZMO_DIR}/Models/*.cpp $${VIZMO_DIR}/Utilities/*.cpp 
HEADERS += $${VIZMO_DIR}/GUI/*.h $${VIZMO_DIR}/Models/*.h $${VIZMO_DIR}/Utilities/*.h $${VIZMO_DIR}/MotionPlanning/*.h
