# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project("AprilTagTrackers")

# Force wxWidgets to build as a static library
set(wxBUILD_SHARED "0")

# Include subprojects, but exclude them from ALL_BUILD 
# as they may have extra unnecessary targets
add_subdirectory("apriltag" EXCLUDE_FROM_ALL)
add_subdirectory("wxWidgets" EXCLUDE_FROM_ALL)
add_subdirectory("openvr" EXCLUDE_FROM_ALL)

# Hacky way to get openvr lib, as the main repo has been broken for months
# TODO: fork openvr to fix build errors
get_directory_property(PLATFORM_NAME DIRECTORY "openvr" DEFINITION "PLATFORM_NAME")
get_directory_property(PROCESSOR_ARCH DIRECTORY "openvr" DEFINITION "PROCESSOR_ARCH")
set(OpenVR_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/${PLATFORM_NAME}${PROCESSOR_ARCH}/openvr_api.lib")
set(OpenVR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openvr/headers")

# Include main project
add_subdirectory("AprilTagTrackers")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)
