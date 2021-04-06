# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

SET (wxBUILD_SHARED "0" CACHE BOOL "")

project ("AprilTagTrackers")

# Include sub-projects.
add_subdirectory("wxWidgets" EXCLUDE_FROM_ALL)
add_subdirectory("apriltag" EXCLUDE_FROM_ALL)
add_subdirectory("AprilTagTrackers")
add_dependencies("AprilTagTrackers" "apriltags")
add_dependencies("AprilTagTrackers" "wxbase" "wxcore" "wxnet")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)
#set_target_properties("AprilTagTrackers" PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS )
