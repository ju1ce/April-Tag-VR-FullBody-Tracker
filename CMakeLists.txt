# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.5)
project("FullBodyTracking")

option(USE_PRECOMP "Use precompiled headers, requires CMake >= 3.16" ON)
option(USE_ASAN "Enable address sanitizer." OFF)

option(BUILD_DEPS "Disable building any dependencies" ON)
option(REBUILD_DEPS "Ensure that the dependencies are rebuilt if modified." OFF)
option(BUILD_DEPS_RELEASE_ONLY "Normally dependencies will build debug and release versions." OFF)
option(BUILD_DEPS_SHARED "Build dependencies as shared instead of static." OFF)
option(BUILD_DEPS_VERBOSE "Forward cmake output when building dependencies, otherwise saved to deps/build/build-deps.cmake.log." OFF)

option(EXPORT_COMPILE_COMMANDS "Generate compile_commands.json for clangd." OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ${EXPORT_COMPILE_COMMANDS})

# Build deps
include("CMake/build-deps.cmake")
set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/deps" CACHE PATH "Location of deps directory.")
set(DEPS_BINARY_DIR "${DEPS_PREFIX}/build" CACHE PATH "Deps build location")
set(DEPS_INSTALL_DIR "${DEPS_PREFIX}/install" CACHE PATH "Deps install location.")

build_deps("Release")
if (NOT ${BUILD_DEPS_RELEASE_ONLY})
    build_deps("Debug")
endif()

# Build project
set(BUILD_SHARED_LIBS ${BUILD_DEPS_SHARED})
if (${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT})
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "Install directory." FORCE)
endif()

add_subdirectory("AprilTagTrackers")
