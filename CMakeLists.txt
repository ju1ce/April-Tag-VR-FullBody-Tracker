# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("AprilTagTrackers")

#only supported for now
SET (PLATFORM_NAME "linux")
SET (PROCESSOR_ARCH "64")

SET (APRILTAG_LIB "${CMAKE_CURRENT_SOURCE_DIR}/apriltag/build/LIBRARY_OUTPUT_DIRECTORY/Release/apriltags.lib" CACHE FILEPATH "Path to apriltags.lib library file." )
SET (APRILTAG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/apriltag" CACHE PATH "Directory of apriltag header files." )
SET (wxBUILD_SHARED "0" CACHE BOOL "")

SET (OPENVR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openvr/headers")
find_library(OPENVR_LIB libopenvr_api HINTS "${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/${PLATFORM_NAME}${PROCESSOR_ARCH}/" NO_DEFAULT_PATH )


# Include sub-projects.
add_subdirectory("wxWidgets" EXCLUDE_FROM_ALL)
add_subdirectory("apriltag" EXCLUDE_FROM_ALL)
add_subdirectory("AprilTagTrackers")
add_dependencies("AprilTagTrackers" "apriltags")
add_dependencies("AprilTagTrackers" "wxbase" "wxcore" "wxnet")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)
#set_target_properties("AprilTagTrackers" PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS )
