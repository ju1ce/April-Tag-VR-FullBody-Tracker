# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.15)
project("AprilTagTrackers" CXX)

# Options set by superproject
option(ENABLE_LTO "Enable link time optimizations for release builds")
option(USE_ASAN "Create an address sanitizer build.")
option(BUILD_SHARED_LIBS "Attempt to link libraries as shared or static.")
option(ENABLE_PS3EYE "Enable ps3eye camera support, Windows only for now.")
option(ENABLE_ASSERT "Enable assertions.")
set(LOG_LEVEL "" CACHE STRING "0 - Nothing, 1 - Fatal, 2 - Throw, 3 - Trace")
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
    MyApp.cpp MyApp.h
    Debug.h

    IPC/IPC.h
    IPC/UNIXSocket.cpp
    IPC/WindowsNamedPipe.cpp)

# main build output
add_executable(AprilTagTrackers ${ATT_SOURCE_FILES})

# Test that this config is allowed
add_custom_target(test-config-stamp
    COMMAND "${CMAKE_COMMAND}" -E cat
    "${CMAKE_CURRENT_BINARY_DIR}/ExternalProjectFiles/stamp/config-$<LOWER_CASE:$<CONFIG>>"
    VERBATIM)
add_dependencies(AprilTagTrackers test-config-stamp)

# Set the root of includes, rather than relative
target_include_directories(AprilTagTrackers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Link to ps3eye target if enabled
if(WIN32 AND ENABLE_PS3EYE)
    find_package(ps3eye REQUIRED ${FIND_LOCAL})
    target_link_libraries(AprilTagTrackers PUBLIC ps3eye::ps3eye)
endif()

# Link libraries defined as targets, includes are automatic
target_link_libraries(AprilTagTrackers PUBLIC
    ${OpenCV_LIBS} wx::wx openvr::openvr apriltag::apriltag)

# Ensure compiler with c++17 language features
target_compile_features(AprilTagTrackers PUBLIC cxx_std_17)

if(ENABLE_ASSERT)
    target_compile_definitions(AprilTagTrackers PUBLIC ATT_ENABLE_ASSERT)
endif()

target_compile_definitions(AprilTagTrackers PUBLIC
    ATT_LOG_LEVEL=${LOG_LEVEL}
    $<$<CONFIG:Debug>:ATT_OVERRIDE_ERROR_HANDLERS>)

# Set cross-platform defines in sources
if(WIN32)
    target_compile_definitions(AprilTagTrackers PUBLIC "OS_WIN=1")
elseif(UNIX)
    target_compile_definitions(AprilTagTrackers PUBLIC "OS_LINUX=1")
else()
    message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}, compilation will fail.")
endif()

# Build with Address Sanitizer
if(USE_ASAN)
    if(MSVC)
        set(ASAN_FLAGS /fsanitize=address)
    else()
        set(ASAN_FLAGS
            -fsanitize=address
            -fsanitize=leak
            -fsanitize=undefined)
    endif()

    target_compile_options(AprilTagTrackers PUBLIC "${ASAN_FLAGS}")
endif()

# Install application to bin folder
install(TARGETS AprilTagTrackers RUNTIME)

# Find the resource file and icon
target_include_directories(AprilTagTrackers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/resources")

if(WIN32)
    # install pdb file for debugging
    install(FILES $<TARGET_PDB_FILE:AprilTagTrackers> DESTINATION "bin" OPTIONAL)

    # set as GUI application on windows
    set_target_properties(AprilTagTrackers PROPERTIES WIN32_EXECUTABLE TRUE)

    # wxWidgets will compile a resource.rc file for us
    target_sources(AprilTagTrackers PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/resource.rc")
endif()

# Make AprilTagTrackers the default target in Visual Studio
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)

# Enable LTO in release
if(ENABLE_LTO)
    set_target_properties(AprilTagTrackers PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()
