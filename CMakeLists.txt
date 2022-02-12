# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.5)
project("FullBodyTracking")

# Build deps
add_subdirectory("deps")

# Build project
set(ATT_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/AprilTagTrackers")
set(ATT_BINARY_DIR "${ATT_PREFIX}/build")
set(ATT_INSTALL_DIR "${ATT_PREFIX}/install"
    CACHE PATH "AprilTagTrackers install directory.")

include(ExternalProject)
ExternalProject_Add(
    AprilTagTrackers
    # Directories
    PREFIX "${ATT_PREFIX}" SOURCE_DIR "${ATT_PREFIX}" DOWNLOAD_DIR "${ATT_PREFIX}"
    BINARY_DIR "${ATT_BINARY_DIR}" INSTALL_DIR "${ATT_INSTALL_DIR}"
    TMP_DIR "${ATT_BINARY_DIR}/cmake-tmp" STAMP_DIR "${ATT_BINARY_DIR}/cmake-tmp"
    # CMake handles the rest
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DDEPS_INSTALL_DIR:PATH=${DEPS_INSTALL_DIR}
        -DBUILD_SHARED_LIBS:BOOL=OFF
    # Deps
    DEPENDS apriltag wxWidgets openvr opencv
)
