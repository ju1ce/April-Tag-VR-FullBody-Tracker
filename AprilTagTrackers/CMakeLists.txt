# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

if(NOT EXISTS ${APRILTAG_LIB} )
  message(FATAL_ERROR "Please set the correct apriltags library." )
endif()

if(NOT EXISTS "${APRILTAG_INCLUDE_DIR}/apriltag.h" )
  message(FATAL_ERROR "Please set the correct apriltags include directory." )
endif()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${APRILTAG_INCLUDE_DIR} )
include_directories( ${OPENVR_INCLUDE_DIR} )

# Add source to this project's executable.
file(GLOB source "*.cpp" "*.h")
add_executable (AprilTagTrackers ${source})

# TODO: Add tests and install targets if needed.

target_link_libraries( AprilTagTrackers ${OPENVR_LIB} )
target_link_libraries( AprilTagTrackers ${OpenCV_LIBS} )
target_link_libraries( AprilTagTrackers ${APRILTAG_LIB} )
target_link_libraries( AprilTagTrackers wx::net wx::core wx::base)
