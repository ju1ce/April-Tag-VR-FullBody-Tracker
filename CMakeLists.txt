cmake_minimum_required(VERSION 3.16)

# cmake options
# required to decide default triplet
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(SUPERPROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/helpers.cmake")

att_default_triplet(ATT_TRIPLET)

# setup vcpkg manifest mode, set before first project() call
set(VCPKG_TARGET_TRIPLET "${ATT_TRIPLET}" CACHE STRING "")
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg-ports")
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg-triplets")
set(VCPKG_INSTALL_OPTIONS "--clean-after-build" CACHE STRING "")

# clone a local vcpkg install if user doesn't already have
# set the toolchain file to vcpkg.cmake
att_bootstrap_vcpkg()

# vcpkg stuff must be done before project
project("April-Tag-VR-FullBody-Tracker" CXX)

# on windows default install is C:\Program Files (x86)\ which requires admin perms
# this sets it for any platform if use doesn't provide
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "Install directory." FORCE)
    message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")
endif()

# multi config generators like Visual Studio and Ninja Multi-Config
get_property(ATT_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT ATT_IS_MULTI_CONFIG)
    # Set default build type to release on single config
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Debug or Release.")
endif()

# cmake options
option(CMAKE_EXPORT_COMPILE_COMMANDS "Create compile_commands.json (not available with VS)" ON)
option(ENABLE_TESTING "Build and install att_test binary" OFF)

# project options
option(ATT_ENABLE_ASAN "Build with address sanitizer" OFF)
option(ATT_ENABLE_ASSERT "Enable ATASSERT in release builds" OFF)
option(ATT_LOG_TO_FILE "Redirect stdout and stderr to output.log" ON)
set(ATT_LOG_LEVEL "3" CACHE STRING "0 - Nothing, 1 - Fatal, 2 - Error, 3 - Status, 4 - Trace")
option(ATT_TEST_ENABLE_ASAN "Build tests with address sanitizer" ON)

# include libs in common/ for every project
add_subdirectory("common" EXCLUDE_FROM_ALL)

att_clone_submodule("BridgeDriver")
att_read_version_file(DRIVER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/BridgeDriver/VERSION.txt")

add_subdirectory("AprilTagTrackers")
add_subdirectory("BridgeDriver")

# folder layout maintained
install(DIRECTORY "utilities" "bindings" "locales" "images-to-print" DESTINATION ".")
