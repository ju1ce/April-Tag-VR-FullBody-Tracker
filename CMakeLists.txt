cmake_minimum_required(VERSION 3.15)

if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Delete build folder if changed.")
endif()
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg-ports")
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/triplets")
set(VCPKG_INSTALL_OPTIONS "--clean-after-build")

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/automate-vcpkg.cmake")
vcpkg_bootstrap()

project("April-Tag-VR-FullBody-Tracker" CXX)

# Fix default install path on windows
# Noone wants to provide admin access and install to C:/Program Files (x86)/
if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(WARNING "On Windows the default install destination is ${CMAKE_INSTALL_PREFIX}, \
which requires admin rights and usually results in errors. \
Explicitly set with -DCMAKE_INSTALL_PREFIX= to install there.")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "Install directory." FORCE)
    message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/helpers.cmake")

# include libs in common/ for every project
add_subdirectory("common" EXCLUDE_FROM_ALL)

att_clone_submodule("BridgeDriver")
att_read_version_file(DRIVER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/BridgeDriver/VERSION.txt")

add_subdirectory("AprilTagTrackers")
