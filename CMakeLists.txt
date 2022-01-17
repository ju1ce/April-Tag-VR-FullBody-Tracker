# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("AprilTagTrackers")

#only supported for now
SET (PLATFORM_NAME "win")
SET (PROCESSOR_ARCH "64")

SET (APRILTAG_LIB "${CMAKE_CURRENT_SOURCE_DIR}/apriltag/build/LIBRARY_OUTPUT_DIRECTORY/Release/apriltags.lib" CACHE FILEPATH "Path to apriltags.lib library file." )
SET (APRILTAG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/apriltag" CACHE PATH "Directory of apriltag header files." )
SET (wxBUILD_SHARED "0" CACHE BOOL "")

SET (OPENVR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openvr/headers")
find_library(OPENVR_LIB openvr_api HINTS "${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/${PLATFORM_NAME}${PROCESSOR_ARCH}/" NO_DEFAULT_PATH )

SET (LIBUSB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libusb-compiled/include/libusb-1.0")
find_library(LIBUSB_LIB libusb-1.0 HINTS "${CMAKE_CURRENT_SOURCE_DIR}/libusb-compiled/VS2019/MS64/dll/" NO_DEFAULT_PATH )

# Include sub-projects.
add_subdirectory("wxWidgets")
add_subdirectory("openvr")
add_subdirectory ("AprilTagTrackers")

#add_subdirectory ("PS3EYEDriver")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)
set_target_properties("AprilTagTrackers" PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS )
