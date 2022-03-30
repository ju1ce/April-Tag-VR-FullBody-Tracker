find_program(NINJA_CMD ninja REQUIRED)
find_program(CLANG_CL_CMD clang-cl REQUIRED)
find_program(LLD_LINK_CMD lld-link REQUIRED)

set(CMAKE_MAKE_PROGRAM "${NINJA_CMD}" CACHE STRING "Set by toolchain" FORCE)
set(CMAKE_GENERATOR "Ninja" CACHE STRING "Set by toolchain" FORCE)

# Fix detect compiler ID
set(MSVC_INCREMENTAL_DEFAULT 1 CACHE STRING "Set by toolchain: Fix compiler detection." FORCE)

set(MSVC_VERSION 1931 CACHE STRING "Set by toolchain" FORCE)
set(MSVC_TOOLSET_VERSION 143 CACHE STRING "Set by toolchain" FORCE)
set(CMAKE_C_COMPILER "${CLANG_CL_CMD}" CACHE STRING "Set by toolchain" FORCE)
set(CMAKE_CXX_COMPILER "${CLANG_CL_CMD}" CACHE STRING "Set by toolchain" FORCE)

set(CMAKE_LINKER "${LLD_LINK_CMD}" CACHE STRING "Set by toolchain" FORCE)

add_compile_options(
    -fuse-ld=lld-link

    -fcolor-diagnostics
    -fmsc-version=${MSVC_VERSION}
    -mssse3
    -msse4.1
    -msse4.2)

if(NOT(PROJECT_NAME STREQUAL "AprilTagTrackers"))
    add_compile_options(/w -Wno-unknown-argument -Wno-unused-command-line-argument)
endif()
