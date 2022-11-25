cmake_minimum_required(VERSION 3.16)
project(debug_driver CXX)

# project output
add_executable(debug_driver)

# ====== Source Files ======

set(ATT_DEBUG_DRIVER_SOURCES
    utils/Env.cpp
    utils/Log.cpp
    Helpers.cpp

    debug_driver/main.cpp
)

if (WIN32)
    list(APPEND ATT_DEBUG_DRIVER_SOURCES
        IPC/WindowsNamedPipe.cpp
    )
else()
    list(APPEND ATT_DEBUG_DRIVER_SOURCES
        IPC/UNIXSocket.cpp
    )
endif()

# prepend the AprilTagTrackers/ base to every source
get_filename_component(ATT_SOURCES_BASE "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
list(TRANSFORM ATT_DEBUG_DRIVER_SOURCES PREPEND "${ATT_SOURCES_BASE}/")
target_sources(debug_driver PRIVATE ${ATT_DEBUG_DRIVER_SOURCES})

# ====== Dependencies ======

find_package(OpenGL REQUIRED)

target_link_libraries(debug_driver PRIVATE
    Threads::Threads
    ${OpenCV_LIBRARIES}
    doctest::doctest
    Taskflow::Taskflow
    common::semver
    opengl32
)

# ====== Compiler Defines ======

target_compile_definitions(debug_driver PRIVATE
    ATT_DRIVER_VERSION=${DRIVER_VERSION}
    ATT_LOG_LEVEL=${ATT_LOG_LEVEL}
    $<$<BOOL:${ATT_DEBUG}>:ATT_DEBUG>
)

att_target_platform_definitions(debug_driver)

# ====== Compiler Options ======

# Set the root of includes, rather than relative
target_include_directories(debug_driver PRIVATE
    "${ATT_SOURCES_BASE}"
)

if (BUILD_SHARED_LIBS)
    att_target_crt_linkage(debug_driver DYNAMIC)
else()
    att_target_crt_linkage(debug_driver STATIC)
endif()

# Ensure compiler with c++20 language features
target_compile_features(debug_driver PRIVATE cxx_std_20)

# Create debug symbols for release builds, msvc will generate a pdb,
# while gcc-like will have embedded symbols.
att_exe_debug_symbols(debug_driver)
att_target_strict_conformance(debug_driver)
if (ATT_DEBUG)
    att_target_enable_diagnostics(debug_driver)
endif()

# Build with Address Sanitizer
if(ATT_TEST_ENABLE_ASAN)
    att_target_enable_asan(debug_driver)
endif()

# Enable LTO
set_target_properties(debug_driver PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# ====== CMake Configuration ======

# Install application to bin folder
install(TARGETS debug_driver RUNTIME DESTINATION ".")

if(WIN32)
    # install pdb file for debugging
    install(FILES $<TARGET_PDB_FILE:debug_driver> DESTINATION "." OPTIONAL)
endif()

# most sources are duplicated, remove from code analysis
set_target_properties(debug_driver PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
