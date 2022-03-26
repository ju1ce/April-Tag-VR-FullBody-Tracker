# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required(VERSION 3.11)
project("April-Tag-VR-FullBody-Tracker" NONE)

# Options
set(USE_ASAN OFF CACHE BOOL "Create an address sanitizer build.")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Attempt to link libraries as shared or static.")
set(EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands for language servers.")
set(WX_DOWNLOAD_MINIMAL ON CACHE BOOL "Download OS specific wxWidgets sources.")

# region Fixes

# The toolchain file doesn't like being absolute
get_filename_component(CMAKE_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)

# Fix default install path on windows
# Noone wants to provide admin access and install to C:/Program Files (x86)/, come on...
if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(FATAL_ERROR "On Windows the default install destination is ${CMAKE_INSTALL_PREFIX}, \
which requires admin rights and usually results in errors. \
Explicitly set with -DCMAKE_INSTALL_PREFIX= to a relative or absolute path. \
Rerun if you are sure about installing to this location.")
endif()

# only defined on multi config
if(CMAKE_CONFIGURATION_TYPES)
    # If using a multi config generator, wxWidgets and opencv for some reason
    # remove the other build types and msbuild errors if we dont do the same.
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "Limited configuration types to fix dependencies." FORCE)

    # VS Gets all weird if we use cmakes built in generator expression??
    # Seems like a new bug in vs 2022
    if(CMAKE_GENERATOR MATCHES "Visual Studio")
        set(CONFIG_GENEXPR $(Configuration))
    else()
        set(CONFIG_GENEXPR $<CONFIG>)
        message(WARNING "Multi config generators other than VS are not tested.")
    endif()

    # Only expands on multi config, single config generators are fine with default commands
    set(EP_DO_MULTI_CONFIG
        BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${CONFIG_GENEXPR}
        INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --prefix <INSTALL_DIR> --config ${CONFIG_GENEXPR})
else()
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Debug or Release.")

    # Only expands on single config, multi config will warn that it is unused
    set(EP_DO_SINGLE_CONFIG -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

# endregion Fixes

# common cmake args for external projects using cmake, used by deps aswell
set(CMAKE_EXPORT_COMPILE_COMMANDS ${EXPORT_COMPILE_COMMANDS})
set(COMMON_CMAKE_ARGS
    ${EP_DO_SINGLE_CONFIG}
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> # gets replaced
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=${CMAKE_EXPORT_COMPILE_COMMANDS}
    -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE})

# Add deps sub-project
set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/deps" CACHE PATH "Location of deps directory.")
set(DEPS_BINARY_DIR "${DEPS_PREFIX}/build" CACHE PATH "Deps build directory.")
set(DEPS_INSTALL_DIR "${DEPS_PREFIX}/install" CACHE PATH "Deps install directory.")
add_subdirectory("${DEPS_PREFIX}" EXCLUDE_FROM_ALL)

set(ENV{GCC_COLORS} "error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01")

# Add ATT external project
include(ExternalProject)
set(ATT_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/AprilTagTrackers")
set(ATT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/AprilTagTrackers")
ExternalProject_Add(
    AprilTagTrackers
    PREFIX "${ATT_BINARY_DIR}"
    BINARY_DIR "${ATT_BINARY_DIR}"
    SOURCE_DIR "${ATT_PREFIX}"
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
    -DDEPS_INSTALL_DIR:PATH=${DEPS_INSTALL_DIR}
    -DUSE_ASAN:BOOL=${USE_ASAN}

    ${EP_DO_MULTI_CONFIG}
    DEPENDS opencv-install wxWidgets-install apriltag-install openvr-install
    BUILD_ALWAYS 1
)

# Make AprilTagTrackers the default target in Visual Studio
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)

# Create merged compilation database
if(CMAKE_EXPORT_COMPILE_COMMANDS AND NOT(CMAKE_GENERATOR MATCHES "Visual Studio"))
    set(db_files "${ATT_BINARY_DIR}/compile_commands.json"
        "${DEPS_BINARY_DIR}/opencv/compile_commands.json"
        "${DEPS_BINARY_DIR}/wxWidgets/compile_commands.json"
        "${DEPS_BINARY_DIR}/apriltag/compile_commands.json")
    set(db_out_file "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
    add_custom_command(TARGET AprilTagTrackers POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        "-DIN_FILES=${db_files}"
        "-DOUT_FILE=${db_out_file}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/scripts/concat-json.cmake"
        BYPRODUCTS ${db_out_file}
        VERBATIM)
endif()
