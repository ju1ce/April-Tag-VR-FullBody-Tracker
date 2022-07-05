if (WIN32)
    option(ATT_DEPS_BUILD_SHARED "Build dependencies shared" OFF)
else()
    option(ATT_DEPS_BUILD_SHARED "Build dependencies shared" ON)
endif()

if (WIN32)
    set(ATT_TARGET_PLATFORM "windows")
elseif(APPLE)
    set(ATT_TARGET_PLATFORM "osx")
elseif(UNIX)
    set(ATT_TARGET_PLATFORM "linux")
endif()

if (ATT_DEPS_BUILD_SHARED)
    set(VCPKG_TARGET_TRIPLET "x64-${ATT_TARGET_PLATFORM}" CACHE STRING "" FORCE)
else()
    set(VCPKG_TARGET_TRIPLET "x64-${ATT_TARGET_PLATFORM}-static" CACHE STRING "" FORCE)
endif()

set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg-ports")
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/triplets")
set(VCPKG_INSTALL_OPTIONS "--clean-after-build" CACHE STRING "")

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/automate-vcpkg.cmake")
vcpkg_bootstrap()

cmake_minimum_required(VERSION 3.16)
project("April-Tag-VR-FullBody-Tracker" CXX)

# Fix default install path on windows
# Noone wants to provide admin access and install to C:/Program Files (x86)/

set(SUPERPROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/shared.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/helpers.cmake")

if (MSVC)
    att_crt_linkage(SHARED ${ATT_DEPS_BUILD_SHARED})
endif()

# include libs in common/ for every project
add_subdirectory("common" EXCLUDE_FROM_ALL)

att_clone_submodule("BridgeDriver")
att_read_version_file(DRIVER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/BridgeDriver/VERSION.txt")

add_subdirectory("AprilTagTrackers")
add_subdirectory("BridgeDriver")

# folder layout maintained
install(DIRECTORY "bindings" "locales" "images-to-print" DESTINATION ".")
