# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)
project("AprilTagTrackers")

# Find prebuilt libraries
find_package(OpenCV REQUIRED 
    opencv_core opencv_imgproc opencv_calib3d
    opencv_features2d opencv_flann opencv_highgui
    opencv_imgcodecs opencv_videoio opencv_videoio_ffmpeg\
    opencv_aruco
    "${CMAKE_SOURCE_DIR}/deps/opencv")


# Add source to this project's executable.
file(GLOB ATT_SOURCE "*.cpp" "*.h" "IPC/*.cpp" "IPC/*.h")
add_executable (AprilTagTrackers ${ATT_SOURCE})
target_include_directories(AprilTagTrackers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Link paths to libraries
target_link_libraries(AprilTagTrackers PUBLIC ${OpenVR_LIB})
target_include_directories(AprilTagTrackers PUBLIC ${OpenVR_INCLUDE_DIR})

# Link libraries defined as targets, includes are automatic
target_link_libraries(AprilTagTrackers PUBLIC ${OpenCV_LIBS} apriltags wx::net wx::core wx::base)

if (MSVC)
    # tell the msvc linker this is a windows application
    set_target_properties(AprilTagTrackers PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
    # useful msvc flags
    target_compile_options(AprilTagTrackers PUBLIC "/MP" "/ZI")
    target_link_options(AprilTagTrackers PUBLIC "/INCREMENTAL")
endif()

# Install target to bin folder
install(TARGETS AprilTagTrackers RUNTIME)
