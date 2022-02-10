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
    DEPENDS apriltag wxWidgets openvr opencv
)
