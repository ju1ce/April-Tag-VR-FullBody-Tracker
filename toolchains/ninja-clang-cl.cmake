find_program(NINJA_CMD ninja REQUIRED)
find_program(CLANG_CL_CMD clang-cl REQUIRED)
find_program(LLD_LINK_CMD lld-link REQUIRED)

set(CMAKE_MAKE_PROGRAM "${NINJA_CMD}" ${CACHE_MSG} CACHE STRING "Set by ninja-clang-cl toolchain." FORCE)
set(CMAKE_GENERATOR "Ninja" ${CACHE_MSG} CACHE STRING "Set by ninja-clang-cl toolchain." FORCE)

# set(ENV{PATH} "${NINJA_CMD}:$ENV{PATH}")

# Fix detect compiler ID
set(MSVC_INCREMENTAL_DEFAULT 1 ${CACHE_MSG} CACHE STRING "Set by ninja-clang-cl toolchain." FORCE)

set(MSVC_VERSION 1931 ${CACHE_MSG} CACHE STRING "Set by ninja-clang-cl toolchain." FORCE)
set(MSVC_TOOLSET_VERSION 143 ${CACHE_MSG} CACHE STRING "Set by ninja-clang-cl toolchain." FORCE)

set(CMAKE_C_COMPILER "${CLANG_CL_CMD}" ${CACHE_MSG} CACHE STRING "Set by ninja-clang-cl toolchain." FORCE)
set(CMAKE_CXX_COMPILER "${CLANG_CL_CMD}" ${CACHE_MSG} CACHE STRING "Set by ninja-clang-cl toolchain." FORCE)

set(CMAKE_LINKER "${LLD_LINK_CMD}" ${CACHE_MSG} CACHE STRING "Set by ninja-clang-cl toolchain." FORCE)

set(DBG_FLAGS
    -gdwarf
    -fdebug-macro)

set(REL_FLAGS
    -flto
    -fvirtual-function-elimination)

add_compile_options(
    "$<$<CONFIG:Debug>:${CLANG_DBG_FLAGS}>"
    "$<$<CONFIG:Release>:${CLANG_REL_FLAGS}>"
    -fuse-ld=lld-link

    -fcolor-diagnostics
    -fms-compatibility
    -fms-compatibility-version=19.31
    -fms-extensions
    -mssse3
    -msse4.1
    -msse4.2)
