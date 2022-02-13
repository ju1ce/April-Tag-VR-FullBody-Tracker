# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.5)
project("FullBodyTracking")

option(USE_PRECOMP "Use precompiled headers, requires CMake >= 3.16" ON)
option(BUILD_DEPS "Ensure that the dependencies are built if modified or empty." ON)
option(BUILD_DEPS_RELEASE_ONLY "Normally dependencies will build debug and release versions." OFF)
option(BUILD_DEPS_SHARED "Build dependencies as shared instead of static." OFF)

# Build deps
include("CMake/build-deps.cmake")
set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/deps" CACHE PATH "Location of deps directory.")
set(DEPS_BINARY_DIR "${DEPS_PREFIX}/build" CACHE PATH "Deps build location")
set(DEPS_INSTALL_DIR "${DEPS_PREFIX}/install" CACHE PATH "Deps install location.")

if (${BUILD_DEPS}) 
    build_deps("Release")
    if (NOT ${BUILD_DEPS_RELEASE_ONLY})
        build_deps("Debug")
    endif()
endif()

if (${DEPS_BUILD_FAILED})
    message(FATAL_ERROR "Failed to build deps.")
    return()
endif()

# Build project
set(BUILD_SHARED_LIBS ${BUILD_DEPS_SHARED})
if (${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT})
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "Install directory." FORCE)
endif()

add_subdirectory("AprilTagTrackers")
