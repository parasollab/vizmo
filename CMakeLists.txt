cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 17)
#add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

project(vizmo)

set(ROBOT_DEF "PMPCfg" CACHE STRING "Robot Type")
add_definitions(-D${ROBOT_DEF})

# used by CMake to scan the header and source files at build time and invoke moc accordingly.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(ppl)

# Add Packages (needs to be installed)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

set(CMAKE_PREFIX_PATH ${QT_ROOT})

#find_package(pmpl REQUIRED)

# Resolve local packages
set(STAPL_PATH ppl/lib/stapl)
find_library(STAPL ${STAPL_PATH})

include_directories(ppl/lib/stapl)
include_directories(ppl/lib/stapl/stapl)

include_directories(${Qt6Widgets_INCLUDE_DIRS})
include_directories(${Qt6OpenGLWidgets_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLUT_INCLUDE_DIRS})

include_directories(ppl/include/mathtool)
include_directories(ppl/lib/gl_visualizer/nonstd)

add_subdirectory(src)
include_directories(src)

# Build Documentation
if(BUILD_DOCS)
    find_package(Doxygen REQUIRED dot OPTIONAL_COMPONENTS mscgen dia)
    add_subdirectory(docs)
endif()

set(LIB lib${PROJECT_NAME})
set(EXEC ${PROJECT_NAME}_exec)

# Build exec
add_executable(${EXEC} src/main.cpp ${VIZMO_SRCS})

target_link_libraries(${EXEC} PRIVATE Qt6::Core Qt6::Gui Qt6::OpenGL Qt6::Widgets Qt6::OpenGLWidgets)
target_link_libraries(${EXEC} PRIVATE ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
target_link_libraries(${EXEC} PRIVATE nonstd)