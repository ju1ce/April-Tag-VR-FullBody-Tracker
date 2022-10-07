cmake_minimum_required(VERSION 3.16)
project(AprilTagTrackers CXX)

# main build output
add_executable(AprilTagTrackers)

# ====== Source Files ======

# sources used in both testing and non testing builds
set(ATT_TESTABLE_SOURCES
    AprilTagWrapper.cpp
    Connection.cpp
    Helpers.cpp
    Quaternion.cpp
    Tracker.cpp
    tagCustom29h10.cpp
    ImageDrawing.cpp

    tracker/OpenVRClient.cpp
    tracker/VRDriver.cpp
    tracker/VideoCapture.cpp

    utils/Env.cpp
    utils/Log.cpp

    serial/FileStorage.cpp
    serial/ReaderWriter.cpp

    ps3eye/ps3eye.cpp
    ps3eye/PSEyeVideoCapture.cpp
)

target_sources(AprilTagTrackers PRIVATE
    ${ATT_TESTABLE_SOURCES}
    MyApp.cpp

    GUI.cpp
    GUI/MainFrame.cpp
    GUI/PreviewPane.cpp
    GUI/FormBuilder.cpp
    GUI/U8String.cpp
    GUI/MatToBitmap.cpp
)

if (WIN32)
    target_sources(AprilTagTrackers PRIVATE
        IPC/WindowsNamedPipe.cpp
    )
elseif(UNIX)
    target_sources(AprilTagTrackers PRIVATE
        IPC/UNIXSocket.cpp
    )
endif()

# ====== Dependencies ======

# find and link dependencies
find_package(Threads REQUIRED)
find_package(wxWidgets REQUIRED core base)
find_package(OpenCV CONFIG REQUIRED)
find_package(libusb CONFIG REQUIRED)
find_package(apriltag CONFIG REQUIRED)
find_package(openvr CONFIG REQUIRED)
find_package(doctest CONFIG REQUIRED)

target_link_libraries(AprilTagTrackers PRIVATE
    Threads::Threads
    ${wxWidgets_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${LIBUSB_LIBRARIES}
    apriltag::apriltag
    openvr::openvr_api
    doctest::doctest
    common::semver
)
target_include_directories(AprilTagTrackers SYSTEM PRIVATE
    ${wxWidgets_INCLUDE_DIRS}
    ${LIBUSB_INCLUDE_DIRS}
)
target_compile_definitions(AprilTagTrackers PRIVATE
    wxDEBUG_LEVEL=$<CONFIG:Debug>
    ${wxWidgets_DEFINITIONS}
    "$<$<CONFIG:Debug>:${wxWidgets_DEFINITIONS_DEBUG}>"
)

# ====== Compiler Defines ======

target_compile_definitions(AprilTagTrackers PRIVATE
    ATT_DRIVER_VERSION=${DRIVER_VERSION}
    ATT_LOG_LEVEL=${ATT_LOG_LEVEL}
    $<$<BOOL:${ATT_DEBUG}>:ATT_DEBUG>
)

att_target_platform_definitions(AprilTagTrackers)

# ====== Compiler Options ======

# Set the root of includes, rather than relative
target_include_directories(AprilTagTrackers PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

if (BUILD_SHARED_LIBS)
    att_target_crt_linkage(AprilTagTrackers DYNAMIC)
else()
    att_target_crt_linkage(AprilTagTrackers STATIC)
endif()

# Ensure compiler with c++20 language features
target_compile_features(AprilTagTrackers PRIVATE cxx_std_20)

# Create debug symbols for release builds, msvc will generate a pdb,
# while gcc-like will have embedded symbols.
att_exe_debug_symbols(AprilTagTrackers)

if (MSVC)
    target_compile_options(AprilTagTrackers PRIVATE
        # all source files are utf8
        /utf-8
        # make msvc slightly more conformant to the standard
        /permissive-
        /Zc:inline
    )
endif()

if (ATT_DEBUG)
    if (MSVC)
        target_compile_options(AprilTagTrackers PRIVATE
            # hide warnings from system includes
            /external:W0
            /external:anglebrackets # any include <> is external
            # enable diagnostics
            /W4
        )
    else()
        target_compile_options(AprilTagTrackers PRIVATE
            # enable diagnostics
            -Wall
            -Wextra
            # warn on non conforming code
            -pedantic
        )
    endif()
endif()

# Build with Address Sanitizer
if(ATT_ENABLE_ASAN)
    att_target_enable_asan(AprilTagTrackers)
endif()

# Precompiled std and library headers
target_precompile_headers(AprilTagTrackers PRIVATE pch.hpp)

# Enable LTO
set_target_properties(AprilTagTrackers PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# ====== CMake Configuration ======

# Install application to bin folder
install(TARGETS AprilTagTrackers RUNTIME DESTINATION ".")

if(WIN32)
    # install pdb file for debugging
    install(FILES $<TARGET_PDB_FILE:AprilTagTrackers> DESTINATION "." OPTIONAL)
    # set as GUI application on windows
    set_target_properties(AprilTagTrackers PROPERTIES WIN32_EXECUTABLE TRUE)
    # wxWidgets will compile a resource.rc file for us
    target_sources(AprilTagTrackers PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/resource.rc")
endif()

# Make AprilTagTrackers the default target in Visual Studio
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)
# setup file structure in IDE
file(GLOB_RECURSE SOURCE_GROUP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.cpp *.c *.tpp)
file(GLOB_RECURSE HEADER_GROUP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.hpp *.h)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SOURCE_GROUP})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HEADER_GROUP})

if (ENABLE_TESTING)
    add_subdirectory("test")
endif()
