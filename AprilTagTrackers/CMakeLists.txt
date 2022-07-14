# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.16)
project(AprilTagTrackers CXX)

option(ENABLE_LTO "Enable link time optimizations for release builds" ON)
option(ENABLE_ASAN "Create an address sanitizer build." OFF)
option(ENABLE_PS3EYE "Enable ps3eye camera support." ON)
option(ENABLE_ASSERT "Enable ATASSERT in release builds." OFF)
option(ENABLE_OUTPUT_LOG_FILE "Redirect stdout and stderr to an output.log file." ON)
set(LOG_LEVEL "2" CACHE STRING "0 - Nothing, 1 - Fatal, 2 - Error, 3 - Trace")

# Explicitly list source files as recommended by cmake
set(ATT_SOURCE_FILES
    AprilTagWrapper.cpp AprilTagWrapper.hpp
    Config.hpp
    Connection.cpp Connection.hpp
    Helpers.cpp Helpers.hpp
    license.hpp
    Localization.hpp
    Quaternion.cpp Quaternion.hpp
    Reflectable.hpp
    Serializable.cpp Serializable.hpp
    ValidatorProxy.hpp
    Tracker.cpp Tracker.hpp
    MyApp.cpp MyApp.hpp
    Debug.cpp Debug.hpp
    RefPtr.hpp

    GUI.cpp GUI.hpp
    GUI/MainFrame.cpp GUI/MainFrame.hpp
    GUI/PreviewPane.cpp GUI/PreviewPane.hpp
    GUI/FormBuilder.cpp GUI/FormBuilder.hpp GUI/FormBuilder.tpp
    GUI/U8String.cpp GUI/U8String.hpp
    GUI/wxHelpers.hpp

    IPC/IPC.hpp
    IPC/UNIXSocket.cpp
    IPC/WindowsNamedPipe.cpp)

# main build output
add_executable(AprilTagTrackers ${ATT_SOURCE_FILES})

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

att_target_link_system_libraries(AprilTagTrackers
    ${OpenCV_LIBRARIES} apriltag::apriltag openvr::openvr_api)
target_link_libraries(AprilTagTrackers PRIVATE
    ${wxWidgets_LIBRARIES})
target_include_directories(AprilTagTrackers SYSTEM PRIVATE
    ${wxWidgets_INCLUDE_DIRS})
target_compile_definitions(AprilTagTrackers PRIVATE
    ${wxWidgets_DEFINITIONS} "$<$<CONFIG:Debug>:${wxWidgets_DEFINITIONS_DEBUG}>")

# Set the root of includes, rather than relative
target_include_directories(AprilTagTrackers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(ENABLE_PS3EYE)
    find_package(libusb CONFIG REQUIRED)
    target_link_libraries(AprilTagTrackers PRIVATE ${LIBUSB_LIBRARIES})
    target_include_directories(AprilTagTrackers SYSTEM PRIVATE ${LIBUSB_INCLUDE_DIRS} ps3eye)
    target_sources(AprilTagTrackers PRIVATE
        ps3eye/ps3eye.cpp
        ps3eye/PSEyeVideoCapture.cpp)

    target_compile_definitions(AprilTagTrackers PUBLIC ATT_ENABLE_PS3EYE)
endif()

target_link_libraries(AprilTagTrackers PUBLIC common::semver)

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

target_compile_definitions(AprilTagTrackers PUBLIC
    ATT_DRIVER_VERSION=${DRIVER_VERSION}
    ATT_LOG_LEVEL=${LOG_LEVEL}
    $<$<CONFIG:Debug>:ATT_OVERRIDE_ERROR_HANDLERS>
    $<$<OR:$<CONFIG:Debug>,$<BOOL:${ENABLE_ASSERT}>>:ATT_ENABLE_ASSERT>
    wxDEBUG_LEVEL=$<IF:$<CONFIG:Release>,0,1>)

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
install(TARGETS AprilTagTrackers RUNTIME DESTINATION ".")

# Find the resource file and icon
target_include_directories(AprilTagTrackers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/resources")

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

# Enable LTO in release
if(ENABLE_LTO)
    set_target_properties(AprilTagTrackers PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

# setup file structure in IDE
file(GLOB_RECURSE SOURCE_GROUP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.cpp *.c)
file(GLOB_RECURSE HEADER_GROUP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.hpp *.h)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SOURCE_GROUP})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HEADER_GROUP})
