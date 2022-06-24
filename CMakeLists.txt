# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required(VERSION 3.15)
project("April-Tag-VR-FullBody-Tracker" CXX)

set(SUPERPROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Where to find, and where to install deps, define before helpers.cmake
set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(DEPS_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/deps/install" CACHE PATH "Deps install directory.")

# Fixes and wrapper functions
include("${SUPERPROJECT_SOURCE_DIR}/CMake/helpers.cmake")

# Build options
option(DEPS_SKIP_BUILD "Skip building deps and use a prebuilt install folder." OFF)
option(DEPS_MINIMAL_BUILD "Remove some unused modules when building dependencies." ON)

if(NOT ATT_IS_MULTI_CONFIG)
    option(EXPORT_COMPILE_COMMANDS "Export compile commands for language servers." ON)
endif()

# AprilTagTrackers forwarded options
AprilTagTrackers_options()

# If not linked to ATT external project, since they are excluded from all, they wont get built.
if(NOT DEPS_SKIP_BUILD)
    # Add deps sub-project
    add_subdirectory("${DEPS_PREFIX}" EXCLUDE_FROM_ALL)
    att_clone_submodule("deps/apriltag")
    att_clone_submodule("deps/opencv")
    att_clone_submodule("deps/opencv_contrib")
    att_clone_submodule("deps/openvr")
    att_clone_submodule("deps/wxWidgets")

    set(ATT_DEPS opencv wxWidgets apriltag openvr)
    # Move the ps3eye files to install, windows only for now
    if(ENABLE_PS3EYE)
        list(APPEND ATT_DEPS ps3eye)
    endif()
endif()

att_add_project(
    BridgeDriver "${CMAKE_INSTALL_PREFIX}"
)
att_clone_submodule("BridgeDriver")

att_add_project(
    AprilTagTrackers "${CMAKE_INSTALL_PREFIX}"
    EXTRA_CMAKE_ARGS
    -DENABLE_ASAN:BOOL=$<BOOL:${ENABLE_ASAN}>
    -DENABLE_PS3EYE:BOOL=$<BOOL:${ENABLE_PS3EYE}>
    -DLOG_LEVEL=${LOG_LEVEL}
    -DENABLE_ASSERT:BOOL=$<BOOL:${ENABLE_ASSERT}>
    -DENABLE_OUTPUT_LOG_FILE:BOOL=$<BOOL:${ENABLE_OUTPUT_LOG_FILE}>

    DEPENDS ${ATT_DEPS}
)

# bindings and locales installed to a subfolder
install(DIRECTORY "bindings" "locales" "images-to-print" DESTINATION ".")


