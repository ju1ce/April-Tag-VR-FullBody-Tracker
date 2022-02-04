# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

# Find prebuilt libraries
find_package(OpenCV REQUIRED)
if (NOT ${OpenCV_opencv_aruco_FOUND})
    message(FATAL_ERROR "Missing Aruco module. Rebuild OpenCV with modules from opencv_contrib")
endif()

# Add source to this project's executable.
file(GLOB source "*.cpp" "*.h" "IPC/*.cpp" "IPC/*.h")
add_executable (AprilTagTrackers ${source})

# Link paths to libraries
target_link_libraries(AprilTagTrackers PUBLIC ${OpenVR_LIB})
target_include_directories(AprilTagTrackers PUBLIC ${OpenVR_INCLUDE_DIR})

# Link libraries defined as targets, includes are automatic
target_link_libraries(AprilTagTrackers PUBLIC ${OpenCV_LIBS} apriltags wx::net wx::core wx::base)

if (MSVC)
    # tell the msvc linker this is a windows application
    set_target_properties(AprilTagTrackers PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
endif()

# Install target to bin folder
install(TARGETS AprilTagTrackers RUNTIME)
