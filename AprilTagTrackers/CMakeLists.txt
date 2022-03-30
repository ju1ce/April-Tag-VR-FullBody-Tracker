# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.11)
project("AprilTagTrackers" CXX)

option(USE_ASAN "Create an address sanitizer build." OFF)
option(BUILD_SHARED_LIBS "Attempt to link libraries as shared or static." OFF)
set(DEPS_INSTALL_DIR "" CACHE PATH "Deps install directory.")

# If using a multi config generator, wxWidgets and opencv due to this random blog post
# remove the other build types and msbuild errors if we dont do the same.
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "Limited configuration types to fix dependencies." FORCE)
endif()

# Search our locally built deps only
if(DEPS_INSTALL_DIR)
    set(FIND_LOCAL PATHS "${DEPS_INSTALL_DIR}" NO_DEFAULT_PATH)
endif()

# Find our dependencies
find_package(OpenCV REQUIRED ${FIND_LOCAL})
find_package(apriltag REQUIRED ${FIND_LOCAL})
find_package(openvr REQUIRED ${FIND_LOCAL})
find_package(wxWidgets REQUIRED core base ${FIND_LOCAL})

# Explicitly list source files as recommended by cmake
set(SOURCE_FILES
    AprilTagWrapper.cpp AprilTagWrapper.h
    Config.h
    Connection.cpp Connection.h
    GUI.cpp GUI.h
    Helpers.cpp Helpers.h
    license.h
    Localization.h
    MessageDialog.cpp MessageDialog.h
    MyApp.cpp MyApp.h
    Quaternion.cpp Quaternion.h
    Reflectable.h
    Serializable.cpp Serializable.h
    Tracker.cpp Tracker.h

    IPC/IPC.h
    IPC/UNIXSocket.cpp
    IPC/WindowsNamedPipe.cpp)

# Main build output, AprilTagTrackers binary
add_executable(AprilTagTrackers ${SOURCE_FILES})

# Set the root of includes, rather than relative
target_include_directories(AprilTagTrackers PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Link libraries defined as targets, includes are automatic
target_link_libraries(AprilTagTrackers PUBLIC
    ${OpenCV_LIBS} wx::wx openvr::openvr apriltag::apriltag)

# Install target to bin folder
install(TARGETS AprilTagTrackers RUNTIME)
install(FILES "$<TARGET_FILE:openvr::openvr>" DESTINATION "bin")

# Ensure compiler with c++17 language features
target_compile_features(AprilTagTrackers PUBLIC cxx_std_17)

# Enable LTO in release
if(NOT USE_ASAN)
    set_target_properties(AprilTagTrackers PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

# Set cross-platform defines in sources
if(WIN32)
    target_compile_definitions(AprilTagTrackers PUBLIC "OS_WIN=1")

    # Link as gui app instead of console app
    set_target_properties(AprilTagTrackers PROPERTIES WIN32_EXECUTABLE ON)
elseif(UNIX)
    target_compile_definitions(AprilTagTrackers PUBLIC "OS_LINUX=1")
else()
    message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}, compilation will fail.")
endif()

set(COMPILER_IS_CLANGCL FALSE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    set(COMPILER_IS_CLANGCL TRUE)
endif()

# Build with Address Sanitizer
if(USE_ASAN)
    if(MSVC)
        set(ASAN_FLAGS /fsanitize=address /Zi)
    else()
        set(ASAN_FLAGS
            -fsanitive=address
            -fsanitive=leak
            -fsanitive=undefined)
    endif()

    target_compile_options(AprilTagTrackers PRIVATE "${ASAN_FLAGS}")
endif()
