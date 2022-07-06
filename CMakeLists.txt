cmake_minimum_required(VERSION 3.16)

set(SUPERPROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/shared.cmake")
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

project("April-Tag-VR-FullBody-Tracker" CXX)

if (MSVC)
    if (BUILD_SHARED_LIBS)
        att_crt_linkage(SHARED)
    else()
        att_crt_linkage(STATIC)
    endif()
endif()

# include libs in common/ for every project
add_subdirectory("common" EXCLUDE_FROM_ALL)

att_clone_submodule("BridgeDriver")
att_read_version_file(DRIVER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/BridgeDriver/VERSION.txt")

add_subdirectory("AprilTagTrackers")
add_subdirectory("BridgeDriver")

# folder layout maintained
install(DIRECTORY "bindings" "locales" "images-to-print" DESTINATION ".")
