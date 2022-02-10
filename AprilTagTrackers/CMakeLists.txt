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
find_package(openvr REQUIRED 
    "${CMAKE_SOURCE_DIR}/deps/openvr")


# Add source to this project's executable.
file(GLOB ATT_SOURCE "*.cpp" "*.h" "IPC/*.cpp" "IPC/*.h")
add_executable (${PROJECT_NAME} ${ATT_SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Link paths to libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenVR_LIB})
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenVR_INCLUDE_DIR})

# Link libraries defined as targets, includes are automatic
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS} apriltags wx::net wx::core wx::base)

if (MSVC)
    # tell the msvc linker this is a windows application
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    # useful msvc flags
    target_compile_options(${PROJECT_NAME} PUBLIC "/MP" "/ZI")
    target_link_options(${PROJECT_NAME} PUBLIC "/INCREMENTAL")
endif()

# Install target to bin folder
install(TARGETS ${PROJECT_NAME} RUNTIME)
