# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

if (UNIX)
    add_definitions (-D__LINUX__)
endif (UNIX)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${APRILTAG_INCLUDE_DIR} )
include_directories( ${OPENVR_INCLUDE_DIR} )

# Add source to this project's executable.
file(GLOB source "*.cpp" "*.h" "*.hpp")
add_executable (AprilTagTrackers ${source})

# TODO: Add tests and install targets if needed.

target_link_libraries( AprilTagTrackers ${OPENVR_LIB} )
target_link_libraries( AprilTagTrackers ${OpenCV_LIBS} )
target_link_libraries( AprilTagTrackers apriltags)
target_link_libraries( AprilTagTrackers wx::base wx::core wx::net)
