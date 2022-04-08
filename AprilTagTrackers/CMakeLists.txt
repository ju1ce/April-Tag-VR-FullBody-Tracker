# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.11)
project("AprilTagTrackers" CXX)

option(USE_ASAN "Create an address sanitizer build." OFF)
option(BUILD_SHARED_LIBS "Attempt to link libraries as shared or static." OFF)
option(ENABLE_PS3EYE "Enable ps3eye camera support, Windows only for now." ON)
set(DEPS_INSTALL_DIR "" CACHE PATH "Deps install directory.")

# If using a multi config generator, wxWidgets and opencv due to this random blog post
# remove the other build types and msbuild errors if we dont do the same.
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "Limited configuration types to fix dependencies." FORCE)
endif()

# Search our locally built deps only
set(FIND_LOCAL PATHS "${DEPS_INSTALL_DIR}" NO_DEFAULT_PATH)

# Find our dependencies
find_package(OpenCV REQUIRED ${FIND_LOCAL})
find_package(apriltag REQUIRED ${FIND_LOCAL})
find_package(openvr REQUIRED ${FIND_LOCAL})
find_package(wxWidgets REQUIRED core base ${FIND_LOCAL})
set(PS3EYE_FILES
    "${DEPS_INSTALL_DIR}/ps3eye/src/ps3eye.cpp"
    "${DEPS_INSTALL_DIR}/ps3eye/include/ps3eye.h"
    "${DEPS_INSTALL_DIR}/ps3eye/src/PSEyeVideoCapture.cpp"
    "${DEPS_INSTALL_DIR}/ps3eye/include/PSEyeVideoCapture.h"
    "${DEPS_INSTALL_DIR}/libusb/include/libusb.h")
set(PS3EYE_INCLUDE_DIRS
    "${DEPS_INSTALL_DIR}/ps3eye/include"
    "${DEPS_INSTALL_DIR}/libusb/include")
set(PS3EYE_LIBS
    "${DEPS_INSTALL_DIR}/libusb/lib/$<CONFIG>/libusb-1.0.lib")

# Explicitly list source files as recommended by cmake
set(ATT_SOURCE_FILES
    AprilTagWrapper.cpp AprilTagWrapper.h
    Config.h
    Connection.cpp Connection.h
    GUI.cpp GUI.h
    Helpers.cpp Helpers.h
    license.h
    Localization.h
    MessageDialog.cpp MessageDialog.h
    Quaternion.cpp Quaternion.h
    Reflectable.h
    Serializable.cpp Serializable.h
    Tracker.cpp Tracker.h

    IPC/IPC.h
    IPC/UNIXSocket.cpp
    IPC/WindowsNamedPipe.cpp)

# Create library of sources (except MyApp which has entry), useful for testing, and separation
add_library(libATT STATIC ${ATT_SOURCE_FILES})

# Set the root of includes, rather than relative
target_include_directories(libATT PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Add the ps3eye driver, video capture, and its libusb dependency
if (WIN32 AND ENABLE_PS3EYE)
    target_compile_definitions(libATT PRIVATE ENABLE_PS3EYE)
    target_link_libraries(libATT PRIVATE ${PS3EYE_LIBS})
    target_include_directories(libATT SYSTEM PRIVATE ${PS3EYE_INCLUDE_DIRS})
    target_sources(libATT PRIVATE ${PS3EYE_FILES})
endif()

# Link libraries defined as targets, includes are automatic
target_link_libraries(libATT PUBLIC
    ${OpenCV_LIBS} wx::wx openvr::openvr apriltag::apriltag)

# Ensure compiler with c++17 language features
target_compile_features(libATT PUBLIC cxx_std_17)

# Enable LTO in release
if(NOT USE_ASAN)
    set_target_properties(libATT PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

# Set cross-platform defines in sources
if(WIN32)
    target_compile_definitions(libATT PUBLIC "OS_WIN=1")
elseif(UNIX)
    target_compile_definitions(libATT PUBLIC "OS_LINUX=1")
else()
    message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}, compilation will fail.")
endif()

# Build with Address Sanitizer
if(USE_ASAN)
    if(MSVC)
        set(ASAN_FLAGS /fsanitize=address)
    else()
        set(ASAN_FLAGS
            -fsanitive=address
            -fsanitive=leak
            -fsanitive=undefined)
    endif()

    target_compile_options(libATT PUBLIC "${ASAN_FLAGS}")
endif()

# Find the resource file and icon
target_include_directories(libATT PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/resources")

# Create the AprilTagTrackers binary
add_executable(AprilTagTrackers MyApp.cpp MyApp.h)
# Link with our configured library
target_link_libraries(AprilTagTrackers PUBLIC libATT)
# Install application to bin folder
install(TARGETS AprilTagTrackers RUNTIME)

if(WIN32)
    # set as GUI application on windows
    set_target_properties(AprilTagTrackers PROPERTIES WIN32_EXECUTABLE TRUE)
    # wxWidgets will compile a resource.rc file for us
    target_sources(AprilTagTrackers PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/resource.rc")
endif()

# Make AprilTagTrackers the default target in Visual Studio
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)
