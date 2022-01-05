# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("AprilTagTrackers")

#only supported for now
SET (PLATFORM_NAME "win")
SET (PROCESSOR_ARCH "64")

SET (APRILTAG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/apriltag" CACHE PATH "Directory of apriltag header files." )
SET (wxBUILD_SHARED "0" CACHE BOOL "")

SET (OPENVR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openvr/headers")
find_library(OPENVR_LIB openvr_api HINTS "${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/${PLATFORM_NAME}${PROCESSOR_ARCH}/" NO_DEFAULT_PATH )

# Include sub-projects.
add_subdirectory("apriltag")
add_subdirectory("wxWidgets")
add_subdirectory("openvr")
add_subdirectory ("AprilTagTrackers")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)
set_target_properties("AprilTagTrackers" PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS )
