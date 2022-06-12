# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.15)
project(AprilTagTrackers CXX)

# Fixes and wrapper functions
include("${SUPERPROJECT_SOURCE_DIR}/CMake/shared.cmake")

# Options set by superproject
AprilTagTrackers_options()

# Find our dependencies
att_find_dep(OpenCV REQUIRED)
att_find_dep(apriltag REQUIRED)
att_find_dep(openvr REQUIRED)
att_find_dep(wxWidgets REQUIRED core base)

# Explicitly list source files as recommended by cmake
set(ATT_SOURCE_FILES
    AprilTagWrapper.cpp AprilTagWrapper.h
    Config.h
    Connection.cpp Connection.h
    Helpers.cpp Helpers.h
    license.h
    Localization.h
    Quaternion.cpp Quaternion.h
    Reflectable.h
    Serializable.cpp Serializable.h
    ValidatorProxy.h
    Tracker.cpp Tracker.h
    MyApp.cpp MyApp.h
    Debug.cpp Debug.h
    RefPtr.h

    GUI.cpp GUI.h
    GUI/MainFrame.cpp GUI/MainFrame.h
    GUI/PreviewPane.cpp GUI/PreviewPane.h
    GUI/FormBuilder.cpp GUI/FormBuilder.h GUI/FormBuilder.tpp
    GUI/U8String.cpp GUI/U8String.h
    GUI/wxHelpers.h

    IPC/IPC.h
    IPC/UNIXSocket.cpp
    IPC/WindowsNamedPipe.cpp)

# main build output
add_executable(AprilTagTrackers ${ATT_SOURCE_FILES})

att_check_config_stamp(AprilTagTrackers)

# Set the root of includes, rather than relative
target_include_directories(AprilTagTrackers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Link to ps3eye target if enabled
if(ENABLE_PS3EYE)
    att_find_dep(ps3eye REQUIRED)
    target_compile_definitions(AprilTagTrackers PUBLIC ATT_ENABLE_PS3EYE)
    target_link_libraries(AprilTagTrackers PUBLIC ps3eye::ps3eye)
endif()

# Link libraries defined as targets, includes are automatic
target_link_libraries(AprilTagTrackers PUBLIC
    ${OpenCV_LIBS} wx::wx openvr::openvr apriltag::apriltag
    common::semver)

# Ensure compiler with c++17 language features
target_compile_features(AprilTagTrackers PUBLIC cxx_std_17)

if(ENABLE_OUTPUT_LOG_FILE)
    target_compile_definitions(AprilTagTrackers PUBLIC ATT_ENABLE_OUTPUT_LOG_FILE)
endif()

# Create debug symbols for release builds, msvc will generate a pdb,
# while gcc-like will have embedded symbols.
if(MSVC)
    # Generates debug symbols in a PDB
    target_compile_options(AprilTagTrackers PRIVATE
        "$<$<CONFIG:Release>:/Zi>")
    # enable debug and re-enable optimizations that it disables
    target_link_options(AprilTagTrackers PRIVATE
        "$<$<CONFIG:Release>:/DEBUG>"
        "$<$<CONFIG:Release>:/OPT:REF>"
        "$<$<CONFIG:Release>:/OPT:ICF>")
    # Set file name and location
    set_target_properties(AprilTagTrackers PROPERTIES
        COMPILE_PDB_NAME AprilTagTrackers
        COMPILE_PDB_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
    )
else()
    target_compile_options(AprilTagTrackers PRIVATE
        $<$<CONFIG:Release>:-g>)
endif()

att_read_version_file(DRIVER_VERSION "${SUPERPROJECT_SOURCE_DIR}/BridgeDriver/VERSION.txt")

target_compile_definitions(AprilTagTrackers PUBLIC
    ATT_DRIVER_VERSION=${DRIVER_VERSION}
    ATT_LOG_LEVEL=${LOG_LEVEL}
    $<$<CONFIG:Debug>:ATT_OVERRIDE_ERROR_HANDLERS>
    $<$<OR:$<CONFIG:Debug>,$<BOOL:${ENABLE_ASSERT}>>:ATT_ENABLE_ASSERT>)

# Set cross-platform defines in sources
if(WIN32)
    target_compile_definitions(AprilTagTrackers PUBLIC "OS_WIN=1")
elseif(UNIX)
    target_compile_definitions(AprilTagTrackers PUBLIC "OS_LINUX=1")
else()
    message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}, compilation will fail.")
endif()

# Build with Address Sanitizer
if(ENABLE_ASAN)
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
