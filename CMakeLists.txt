# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

# TODO: Oldest tested version, update if you test an older version.
cmake_minimum_required(VERSION 3.16)
project("April-Tag-VR-FullBody-Tracker" NONE)

# Fix default install path on windows
# Noone wants to provide admin access and install to C:/Program Files (x86)/, come on...
if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(FATAL_ERROR "On Windows the default install destination is ${CMAKE_INSTALL_PREFIX}, \
which requires admin rights and usually results in errors. \
Explicitly set with -DCMAKE_INSTALL_PREFIX= to a relative or absolute path. \
Rerun if you are sure about installing to this location.")
endif()

# Options
option(USE_ASAN "Create an address sanitizer build." OFF)
option(BUILD_SHARED_LIBS "Attempt to link libraries as shared or static." OFF)
option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compile commands for language servers." OFF)

# common cmake args for external projects using cmake
set(COMMON_CMAKE_ARGS
    -Wno-dev # Suppress dependency warnings we can't fix
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> # gets replaced
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=${CMAKE_EXPORT_COMPILE_COMMANDS})

# Add deps sub-project
set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/deps" CACHE PATH "Location of deps directory.")
set(DEPS_BINARY_DIR "${DEPS_PREFIX}/build" CACHE PATH "Deps build directory.")
set(DEPS_INSTALL_DIR "${DEPS_PREFIX}/install" CACHE PATH "Deps install directory.")
add_subdirectory("${DEPS_PREFIX}" "${DEPS_BINARY_DIR}" EXCLUDE_FROM_ALL)

# Add ATT external project
include(ExternalProject)
ExternalProject_Add(
    AprilTagTrackers
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/AprilTagTrackers"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/AprilTagTrackers"
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/AprilTagTrackers"
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
        -DDEPS_INSTALL_DIR:PATH=${DEPS_INSTALL_DIR}
        -DUSE_ASAN:BOOL=${USE_ASAN}
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    DEPENDS opencv apriltag openvr wxWidgets
    BUILD_ALWAYS 1
)

# Make AprilTagTrackers the default target in Visual Studio
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)

# Create merged compilation database
if (CMAKE_EXPORT_COMPILE_COMMANDS)
    set(db_files
        "${DEPS_BINARY_DIR}/apriltag/compile_commands.json"
        "${DEPS_BINARY_DIR}/opencv/compile_commands.json"
        "${DEPS_BINARY_DIR}/wxWidgets/compile_commands.json"
        "${CMAKE_CURRENT_BINARY_DIR}/AprilTagTrackers/compile_commands.json")
    set(db_out_file "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
    add_custom_command(TARGET AprilTagTrackers POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            "-DIN_FILES=${db_files}"
            "-DOUT_FILE=${db_out_file}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/scripts/concat-json.cmake"
        BYPRODUCTS ${db_out_file}
        VERBATIM)
endif()