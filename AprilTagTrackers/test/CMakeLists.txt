cmake_minimum_required(VERSION 3.16)
project(test CXX)

# project output
add_executable(test)

# ====== Source Files ======

# base dir of sources is AprilTagTrackers/
set(ATT_TEST_SOURCES
    AprilTagWrapper.cpp
    Connection.cpp
    Helpers.cpp
    Quaternion.cpp
    Serializable.cpp
    Tracker.cpp
    tagCustom29h10.cpp

    utils/Env.cpp
    utils/Log.cpp

    ps3eye/ps3eye.cpp
    ps3eye/PSEyeVideoCapture.cpp

    test/main.cpp
    test/GUI_stub.cpp
)

if (WIN32)
    list(APPEND ATT_TEST_SOURCES
        IPC/WindowsNamedPipe.cpp
    )
else()
    list(APPEND ATT_TEST_SOURCES
        IPC/UNIXSocket.cpp
    )
endif()

# prepend the AprilTagTrackers/ base to every source
get_filename_component(ATT_SOURCES_BASE "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
list(TRANSFORM ATT_TEST_SOURCES PREPEND "${ATT_SOURCES_BASE}/")
target_sources(test PRIVATE ${ATT_TEST_SOURCES})

# ====== Dependencies ======

target_link_libraries(test PRIVATE
    Threads::Threads
    ${OpenCV_LIBRARIES}
    ${LIBUSB_LIBRARIES}
    apriltag::apriltag
    openvr::openvr_api
    doctest::doctest
    common::semver
)
target_include_directories(test SYSTEM PRIVATE
    ${LIBUSB_INCLUDE_DIRS}
)
# wxWidgets isn't linked, but we want the definitions to match somewhat
target_compile_definitions(test PRIVATE
    wxDEBUG_LEVEL=$<CONFIG:Debug>
    ${wxWidgets_DEFINITIONS}
    "$<$<CONFIG:Debug>:${wxWidgets_DEFINITIONS_DEBUG}>"
)

# ====== Compiler Defines ======

target_compile_definitions(test PRIVATE
    ATT_DRIVER_VERSION=${DRIVER_VERSION}
    ATT_LOG_LEVEL=${ATT_LOG_LEVEL}
    $<$<BOOL:${ATT_DEBUG}>:ATT_DEBUG>
    ATT_TESTING
)

att_target_platform_definitions(test)

# ====== Compiler Options ======

# Set the root of includes, rather than relative
target_include_directories(test PRIVATE
    "${ATT_SOURCES_BASE}"
)

if (BUILD_SHARED_LIBS)
    att_target_crt_linkage(test DYNAMIC)
else()
    att_target_crt_linkage(test STATIC)
endif()

# Ensure compiler with c++17 language features
target_compile_features(test PRIVATE cxx_std_17)

# Create debug symbols for release builds, msvc will generate a pdb,
# while gcc-like will have embedded symbols.
att_exe_debug_symbols(test)

if (MSVC)
    target_compile_options(AprilTagTrackers PRIVATE
        # all source files are utf8
        /utf-8
        # make msvc slightly more conformant to the standard
        /permissive-
        /Zc:inline
    )
endif()

# Build with Address Sanitizer
if(ATT_TEST_ENABLE_ASAN)
    att_target_enable_asan(test)
endif()

# precompiled headers
target_precompile_headers(test PRIVATE "${ATT_SOURCES_BASE}/pch.hpp")

# Enable LTO
set_target_properties(AprilTagTrackers PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# ====== CMake Configuration ======

# Install application to bin folder
install(TARGETS test RUNTIME DESTINATION ".")

if(WIN32)
    # install pdb file for debugging
    install(FILES $<TARGET_PDB_FILE:test> DESTINATION "." OPTIONAL)
endif()

# most sources are duplicated, remove from code analysis
set_target_properties(test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
