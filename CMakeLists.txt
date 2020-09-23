cmake_minimum_required(VERSION 2.8.11)

project(solar_qt)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
find_package(OpenGL)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

SET(QtApp_RCCS Icons.qrc)
QT5_ADD_RESOURCES(QtApp_RCC_SRCS ${QtApp_RCCS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Tell CMake to create the helloworld executable
add_executable(solarvis main.cpp base.h base.cpp solarviswindow.h solarviswindow.cpp solarsystem.h
    solarsystem.cpp planet.h planet.cpp sun.h sun.cpp utils.h utils.cpp vec3d.h vec3d.cpp
    ${QtApp_RCC_SRCS} solarviswindow.ui
    basicmode.h basicmode.cpp basicmode.ui
    realmode.h realmode.cpp realmode.ui)

# Use the Widgets module from Qt 5.
target_link_libraries(solarvis Qt5::Widgets Qt5::OpenGL ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${VTK_LIBRARIES})
