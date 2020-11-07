# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

SET (APRILTAG_LIB "${CMAKE_CURRENT_SOURCE_DIR}/apriltag/build/LIBRARY_OUTPUT_DIRECTORY/Release/apriltags.lib" CACHE FILEPATH "Path to apriltags.lib library file." )
SET (APRILTAG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/apriltag" CACHE PATH "Directory of apriltag header files." )
SET (wxBUILD_SHARED "0" CACHE BOOL "")


project ("AprilTagTrackers")

# Include sub-projects.
add_subdirectory("wxWidgets")
add_subdirectory ("AprilTagTrackers")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)
set_target_properties("AprilTagTrackers" PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS )
