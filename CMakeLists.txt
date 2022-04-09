# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required(VERSION 3.15)
project("April-Tag-VR-FullBody-Tracker" CXX)

# AprilTagTrackers forwarded options
option(USE_ASAN "Create an address sanitizer build." OFF)
option(ENABLE_PS3EYE "Enable ps3eye camera support, Windows only for now." ON)
option(ENABLE_ASSERT "Enable assertions." OFF)
set(LOG_LEVEL 0 CACHE STRING "0 - Nothing, 1 - Fatal, 2 - Throw, 3 - Trace")

# Build options
option(BUILD_SHARED_LIBS "Attempt to link libraries as shared or static." OFF)
option(EXPORT_COMPILE_COMMANDS "Export compile commands for language servers." ON)
option(SKIP_BUILD_DEPS "Skip building deps and use a prebuilt install folder." OFF)

# Fix default install path on windows
# Noone wants to provide admin access and install to C:/Program Files (x86)/
if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(FATAL_ERROR "On Windows the default install destination is ${CMAKE_INSTALL_PREFIX}, \
which requires admin rights and usually results in errors. \
Explicitly set with -DCMAKE_INSTALL_PREFIX= to a relative or absolute path. \
Rerun if you are sure about installing to this location.")
endif()

# Where to find, and where to install deps
set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(DEPS_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/deps/install" CACHE PATH "Deps install directory.")

# Option fixes and wrapper functions
set(CUSTOM_CMAKE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
include("${CUSTOM_CMAKE_FILES}/helpers.cmake")

# Add deps sub-project
add_subdirectory("${DEPS_PREFIX}" EXCLUDE_FROM_ALL)

# If not linked to ATT external project, since they are excluded from all, they wont get built.
if(NOT SKIP_BUILD_DEPS)
    set(ATT_DEPS opencv-install wxWidgets-install apriltag-install openvr-install)
endif()

# Move the ps3eye files to install, windows only for now
if(WIN32 AND ENABLE_PS3EYE)
    list(APPEND ATT_DEPS ps3eye-install)
endif()

att_add_external_project(
    AprilTagTrackers
    EXTRA_CMAKE_ARGS
    -DDEPS_INSTALL_DIR:PATH=${DEPS_INSTALL_DIR}
    -DUSE_ASAN:BOOL=${USE_ASAN}
    -DENABLE_PS3EYE=${ENABLE_PS3EYE}
    -DENABLE_ASSERT=${ENABLE_ASSERT}
    -DLOG_LEVEL=${LOG_LEVEL}

    EXTRA_EP_ARGS
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    BUILD_ALWAYS 1
    DEPENDS ${ATT_DEPS}
)

att_add_install_compile_commands_step(AprilTagTrackers "${CMAKE_CURRENT_SOURCE_DIR}/build")
