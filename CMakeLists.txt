# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project("FullBodyTracking")

# Build deps
add_subdirectory("deps")

set(ATT_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/AprilTagTrackers")
set(ATT_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/AprilTagTrackers")
set(ATT_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/install/AprilTagTrackers"
    CACHE PATH "AprilTagTrackers install directory.")

include(ExternalProject)
ExternalProject_Add(
    AprilTagTrackers
    # Directories
    PREFIX "${ATT_PREFIX}" SOURCE_DIR "${ATT_PREFIX}" DOWNLOAD_DIR "${ATT_PREFIX}"
    BINARY_DIR "${ATT_BINARY_DIR}"
    INSTALL_DIR "${ATT_INSTALL_DIR}"
    TMP_DIR "${ATT_BINARY_DIR}/cmake-tmp" STAMP_DIR "${ATT_BINARY_DIR}/cmake-stamp"
    # Nothing to download
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    # CMake will handle everything else
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    # Deps
    DEPENDS apriltag wxWidgets openvr
)

# Include subprojects, but exclude them from ALL_BUILD 
# as they may have extra unnecessary targets
# add_subdirectory("apriltag" EXCLUDE_FROM_ALL)
# add_subdirectory("wxWidgets" EXCLUDE_FROM_ALL)
# add_subdirectory("openvr" EXCLUDE_FROM_ALL)

# Hacky way to get openvr lib, as the main repo has been broken for months
# TODO: fork openvr to fix build errors
# get_directory_property(PLATFORM_NAME DIRECTORY "openvr" DEFINITION "PLATFORM_NAME")
# get_directory_property(PROCESSOR_ARCH DIRECTORY "openvr" DEFINITION "PROCESSOR_ARCH")
# find_library(OpenVR_LIB openvr_api REQUIRED PATHS "${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/${PLATFORM_NAME}${PROCESSOR_ARCH}" NO_DEFAULT_PATH)
# set(OpenVR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openvr/headers")

# Include main project
# add_subdirectory("AprilTagTrackers")

# set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Test)
