# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required(VERSION 3.15)
project("April-Tag-VR-FullBody-Tracker" CXX)

# Where to find, and where to install deps, define before helpers.cmake
set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(DEPS_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/deps/install" CACHE PATH "Deps install directory.")

# Fixes and wrapper functions
set(CUSTOM_CMAKE_FILES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
include("${CUSTOM_CMAKE_FILES_DIR}/helpers.cmake")

# Build options
option(SKIP_BUILD_DEPS "Skip building deps and use a prebuilt install folder." OFF)

if(NOT ATT_IS_MULTI_CONFIG)
    option(EXPORT_COMPILE_COMMANDS "Export compile commands for language servers." ON)
endif()

# Add deps sub-project
add_subdirectory("${DEPS_PREFIX}" EXCLUDE_FROM_ALL)

# If not linked to ATT external project, since they are excluded from all, they wont get built.
if(NOT SKIP_BUILD_DEPS)
    set(ATT_DEPS opencv-install wxWidgets-install apriltag-install openvr-install)

    # Move the ps3eye files to install, windows only for now
    if(ENABLE_PS3EYE)
        list(APPEND ATT_DEPS ps3eye-install)
    endif()
endif()

# AprilTagTrackers forwarded options
AprilTagTrackers_options()

# If log level isnt defined, default to 3 (trace) in debug, and 2 (error) in release
if(LOG_LEVEL STREQUAL "")
    set(LOG_LEVEL_FLAG $<IF:$<CONFIG:Debug>,3,2>)
else()
    set(LOG_LEVEL_FLAG ${LOG_LEVEL})
endif()

att_add_project(
    AprilTagTrackers
    EXTRA_CMAKE_ARGS
    -DUSE_ASAN:BOOL=$<BOOL:${USE_ASAN}>
    -DENABLE_PS3EYE:BOOL=$<BOOL:${ENABLE_PS3EYE}>
    -DLOG_LEVEL=${LOG_LEVEL_FLAG}
    -DENABLE_LTO:BOOL=$<AND:$<CONFIG:Release>,$<BOOL:${ENABLE_LTO}>> # (CONFIG == Release) and ENABLE_LTO
    -DENABLE_ASSERT:BOOL=$<OR:$<CONFIG:Debug>,$<BOOL:${ENABLE_ASSERT}>> # (CONFIG == Debug) or ENABLE_ASSERT
    -DENABLE_OUTPUT_LOG_FILE:BOOL=$<BOOL:${ENABLE_OUTPUT_LOG_FILE}>

    DEPENDS ${ATT_DEPS}
)
