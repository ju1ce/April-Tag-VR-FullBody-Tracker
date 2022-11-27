cmake_minimum_required(VERSION 3.16)

# cmake options
# required to decide default triplet
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

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
option(ENABLE_TESTING "Build and install test binary" OFF)

# project options
option(ATT_INSTALL_RESOURCES "Copy various required resources to install" ON)
option(ATT_ENABLE_DRIVER "Build and install the BridgeDriver" ON)
option(ATT_ENABLE_DEBUG_DRIVER "Build and install a debug/testing driver" OFF)
option(ATT_DEBUG "Developer mode. Enable custom assert, debug logging, and debugger support. Can be used in release build." OFF)
option(ATT_ENABLE_ANALYZER "Enable compiler static analyzers with ATT_DEBUG. CPU intensive." OFF)
option(ATT_ENABLE_ASAN "Build with address sanitizer" OFF)
set(ATT_LOG_LEVEL "1" CACHE STRING "0 - Silent, 1 - Info, 2 - Debug")
option(ATT_TEST_ENABLE_ASAN "Build tests with address sanitizer" ON)

# include libs in common/ for every project
add_subdirectory("common" EXCLUDE_FROM_ALL)

att_clone_submodule("BridgeDriver")
att_read_version_file(DRIVER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/BridgeDriver/VERSION.txt")

add_subdirectory("AprilTagTrackers")
if (ATT_ENABLE_DRIVER)
    add_subdirectory("BridgeDriver")
endif()

# folder layout maintained
if (ATT_INSTALL_RESOURCES)
    install(DIRECTORY "utilities" "bindings" "locales" "images-to-print" DESTINATION ".")
endif()
