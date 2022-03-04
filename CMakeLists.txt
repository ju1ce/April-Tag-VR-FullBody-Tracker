# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.6)
project("April-Tag-VR-FullBody-Tracker")

# Fix default install path on windows
# Noone wants to provide admin access and install to C:/Program Files (x86)/, come on...
if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(FATAL_ERROR "On Windows the default install destination is ${CMAKE_INSTALL_PREFIX}, \
which requires admin rights and usually results in errors. \
Explicitly set with -DCMAKE_INSTALL_PREFIX= to a relative or absolute path. \
Rerun if you are sure about installing to this location.")
endif()

# Options
option(USE_ASAN "Create an address sanitizer build." OFF)
option(BUILD_SHARED_LIBS "Attempt to link libraries as shared or static." OFF)
option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compile commands for language servers." OFF)

# Add deps sub-project
set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/deps" CACHE PATH "Location of deps directory.")
set(DEPS_BINARY_DIR "${DEPS_PREFIX}/build" CACHE PATH "Deps build directory.")
set(DEPS_INSTALL_DIR "${DEPS_PREFIX}/install" CACHE PATH "Deps install directory.")
add_subdirectory("${DEPS_PREFIX}" "${DEPS_BINARY_DIR}")

# Add ATT external project
include(ExternalProject)
ExternalProject_Add(
    AprilTagTrackers
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/AprilTagTrackers"
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/AprilTagTrackers"
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DDEPS_INSTALL_DIR:PATH=${DEPS_INSTALL_DIR}
        -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=${CMAKE_EXPORT_COMPILE_COMMANDS}
        -DUSE_ASAN:BOOL=${USE_ASAN}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    DEPENDS opencv apriltag openvr wxWidgets
)

# Make AprilTagTrackers the default target in Visual Studio
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)