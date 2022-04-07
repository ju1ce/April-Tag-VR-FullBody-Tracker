# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

# Find prebuilt libraries
find_package(OpenCV REQUIRED)
if (NOT ${OpenCV_opencv_aruco_FOUND})
    message(FATAL_ERROR "Missing Aruco module. Rebuild OpenCV with modules from opencv_contrib")
endif()

if(NOT EXISTS ${LIBUSB_LIB} )
  message(FATAL_ERROR "Please set the correct LIBUSB library." )
endif()

if(NOT EXISTS "${LIBUSB_INCLUDE_DIR}/libusb.h" )
  message(FATAL_ERROR "Please set the correct LIBUSB include directory.")
endif()

# Add source to this project's executable.
file(GLOB source "*.cpp" "*.h" "ps3eyed/*.cpp" "ps3eyed/*.h" "GUI.rc" "IPC/*.cpp" "IPC/*.h")
add_executable (AprilTagTrackers ${source})

file(COPY ${CMAKE_CURRENT_LIST_DIR}/april.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)

# Link paths to libraries
target_link_libraries(AprilTagTrackers PUBLIC ${OpenVR_LIB} ${LIBUSB_LIB})
target_include_directories(AprilTagTrackers PUBLIC ${OpenVR_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})

# Link libraries defined as targets, includes are automatic
target_link_libraries(AprilTagTrackers PUBLIC ${OpenCV_LIBS} apriltags wx::net wx::core wx::base)

if (MSVC)
    # tell the msvc linker this is a windows application
    set_target_properties(AprilTagTrackers PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
endif()

# Install target to bin folder
install(TARGETS AprilTagTrackers RUNTIME)
install(FILES "${AprilTagTrackers_SOURCE_DIR}/april.ico" DESTINATION bin)

#target_link_libraries( AprilTagTrackers ${LIBUSB_LIB} )