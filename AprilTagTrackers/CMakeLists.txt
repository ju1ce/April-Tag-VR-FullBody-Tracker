# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.16)
project(AprilTagTrackers CXX)

# main build output
add_executable(AprilTagTrackers)

target_sources(AprilTagTrackers PRIVATE
    AprilTagWrapper.cpp
    Connection.cpp
    Helpers.cpp
    Quaternion.cpp
    Serializable.cpp
    Tracker.cpp
    MyApp.cpp
    Debug.cpp

    GUI.cpp
    GUI/MainFrame.cpp
    GUI/PreviewPane.cpp
    GUI/FormBuilder.cpp
    GUI/U8String.cpp

    ps3eye/ps3eye.cpp
    ps3eye/PSEyeVideoCapture.cpp
)

if (WIN32)
    target_sources(AprilTagTrackers PRIVATE
        IPC/WindowsNamedPipe.cpp
    )
else()
    target_sources(AprilTagTrackers PRIVATE
        IPC/UNIXSocket.cpp
    )
endif()

if (MSVC)
    if (BUILD_SHARED_LIBS)
        att_target_crt_linkage(AprilTagTrackers DYNAMIC)
    else()
        att_target_crt_linkage(AprilTagTrackers STATIC)
    endif()
endif()

# find and link dependencies
find_package(OpenCV CONFIG REQUIRED)
find_package(apriltag CONFIG REQUIRED)
find_package(wxWidgets REQUIRED core base)
find_package(openvr CONFIG REQUIRED)
find_package(doctest CONFIG REQUIRED)
find_package(libusb CONFIG REQUIRED)

target_link_libraries(AprilTagTrackers PRIVATE
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
    "${CMAKE_CURRENT_SOURCE_DIR}/ps3eye"
)
target_compile_definitions(AprilTagTrackers PRIVATE
    "wxDEBUG_LEVEL=$<IF:$<CONFIG:Release>,0,1>"
    ${wxWidgets_DEFINITIONS}
    "$<$<CONFIG:Debug>:${wxWidgets_DEFINITIONS_DEBUG}>"
    DOCTEST_CONFIG_DISABLE
)

# Set the root of includes, rather than relative
target_include_directories(AprilTagTrackers PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
)

# Ensure compiler with c++17 language features
target_compile_features(AprilTagTrackers PRIVATE cxx_std_17)

if(ATT_LOG_TO_FILE)
    target_compile_definitions(AprilTagTrackers PRIVATE ATT_LOG_TO_FILE)
endif()

# Create debug symbols for release builds, msvc will generate a pdb,
# while gcc-like will have embedded symbols.
att_exe_debug_symbols(AprilTagTrackers)

target_compile_definitions(AprilTagTrackers PRIVATE
    ATT_DRIVER_VERSION=${DRIVER_VERSION}
    ATT_LOG_LEVEL=${ATT_LOG_LEVEL}
    $<$<CONFIG:Debug>:ATT_OVERRIDE_ERROR_HANDLERS>
    $<$<OR:$<CONFIG:Debug>,$<BOOL:${ATT_ENABLE_ASSERT}>>:ATT_ENABLE_ASSERT>
    ATT_ENABLE_PS3EYE
)

# Set cross-platform defines in sources
if(WIN32)
    target_compile_definitions(AprilTagTrackers PRIVATE "OS_WIN=1")
elseif(UNIX)
    target_compile_definitions(AprilTagTrackers PRIVATE "OS_LINUX=1")
else()
    message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}, compilation will fail.")
endif()

# Build with Address Sanitizer
if(ATT_ENABLE_ASAN)
    att_target_enable_asan(AprilTagTrackers)
endif()

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

# Enable LTO
set_target_properties(AprilTagTrackers PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# Make AprilTagTrackers the default target in Visual Studio
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AprilTagTrackers)
# setup file structure in IDE
file(GLOB_RECURSE SOURCE_GROUP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.cpp *.c)
file(GLOB_RECURSE HEADER_GROUP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.hpp *.h)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SOURCE_GROUP})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HEADER_GROUP})
