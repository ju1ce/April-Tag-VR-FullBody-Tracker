include(ExternalProject)
include(CMakePackageConfigHelpers)

# Prefix every global define with ATT/att
# Use functions instead of macros when using local variables to not polute the global scope

# clone a local vcpkg install if user doesn't already have
# set the toolchain file to vcpkg.cmake
function(att_bootstrap_vcpkg)
    # custom toolchain and first run
    if (DEFINED CMAKE_TOOLCHAIN_FILE AND NOT DEFINED VCPKG_ROOT)
        message(STATUS "using custom toolchain, include vcpkg to build dependencies.")
        return()
    endif()

    if (DEFINED ENV{VCPKG_ROOT})
        set(vcpkg_default_root "$ENV{VCPKG_ROOT}")
    elseif (WIN32)
        set(vcpkg_default_root "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")
    else()
        set(vcpkg_default_root "${CMAKE_CURRENT_SOURCE_DIR}/.vcpkg")
    endif()

    set(VCPKG_ROOT "${vcpkg_default_root}" CACHE PATH "vcpkg root directory")
    message(STATUS "vcpkg root: ${VCPKG_ROOT}")

    if (WIN32)
        set(vcpkg_cmd "${VCPKG_ROOT}/${vcpkg_cmd}")
        set(vcpkg_bootstrap_cmd "${VCPKG_ROOT}/bootstrap-vcpkg.bat")
    else()
        set(vcpkg_cmd "${VCPKG_ROOT}/${vcpkg_cmd}")
        set(vcpkg_bootstrap_cmd "${VCPKG_ROOT}/bootstrap-vcpkg.sh")
    endif()

    if (NOT EXISTS "${vcpkg_bootstrap_cmd}")
        find_program(GIT_CMD git REQUIRED)
        execute_process(COMMAND "${GIT_CMD}" clone --filter=tree:0 "https://github.com/microsoft/vcpkg.git" "${VCPKG_ROOT}")

        if (NOT EXISTS "${vcpkg_bootstrap_cmd}")
            message(FATAL_ERROR "failed to clone vcpkg")
        endif()
    endif()

    if (NOT EXISTS "${vcpkg_cmd}")
        execute_process(COMMAND "${vcpkg_bootstrap_cmd}" -disableMetrics
            WORKING_DIRECTORY "${VCPKG_ROOT}")

        if (NOT EXISTS "${vcpkg_cmd}")
            message(FATAL_ERROR "failed to bootstrap vcpkg")
        endif()
    endif()

    if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
endfunction()

# figure out a vcpkg triplet for this platform
# override by setting cache variable VCPKG_TARGET_TRIPLET
function(att_default_triplet out_triplet)
    set(target_arch "x64")
    if (WIN32)
        set(triplet "${target_arch}-windows")
        if (NOT BUILD_SHARED_LIBS)
            set(triplet "${triplet}-static")
        endif()
    elseif(UNIX)
        if (APPLE)
            set(triplet "${target_arch}-osx")
        else()
            set(triplet "${target_arch}-linux")
        endif()
        if (BUILD_SHARED_LIBS)
            set(triplet "${triplet}-dynamic")
        endif()
    endif()

    set(${out_triplet} "${triplet}" PARENT_SCOPE)
endfunction()

# adds definitions ATT_OS_<NAME> and ATT_COMP_<NAME>
function(att_target_platform_definitions target)
    if (WIN32)
        set(os_name "WINDOWS")
    elseif(APPLE)
        set(os_name "MACOS")
    elseif(UNIX)
        set(os_name "LINUX")
    else()
        set(os_name "UNKNOWN")
    endif()

    if (MSVC)
        set(comp_name "MSVC")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(comp_name "CLANG")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(comp_name "GCC")
    else()
        set(comp_name "UNKNOWN")
    endif()

    target_compile_definitions(${target} PRIVATE
        ATT_OS_${os_name}
        ATT_COMP_${comp_name}
    )
endfunction()

# Set the CRT linkage on windows, by setting MSVC_RUNTIME_LIBRARY property
# set STATIC (/MT) or DYNAMIC (/MD)
function(att_target_crt_linkage target)
    cmake_parse_arguments(PARSE_ARGV 1 _arg "STATIC;DYNAMIC" "" "")
    if (NOT target)
        message(FATAL_ERROR "Argument target required")
    endif()

    if (_arg_DYNAMIC)
        set_target_properties(${target} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    elseif(_arg_STATIC)
        set_target_properties(${target} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        message(FATAL_ERROR "Argument STATIC or DYNAMIC required")
    endif()
endfunction()

# Create debug symbols for release builds, msvc will generate a pdb,
# while gcc-like will have embedded symbols.
function(att_exe_debug_symbols target)
    if(MSVC)
        # Generates debug symbols in a PDB
        target_compile_options(${target} PRIVATE
            "$<$<CONFIG:Release>:/Zi>")
        # enable debug and re-enable optimizations that it disables
        target_link_options(${target} PRIVATE
            "$<$<CONFIG:Release>:/DEBUG>"
            "$<$<CONFIG:Release>:/OPT:REF>"
            "$<$<CONFIG:Release>:/OPT:ICF>")
        # Set file name and location
        set_target_properties(${target} PROPERTIES
            COMPILE_PDB_NAME "${target}"
            COMPILE_PDB_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    else()
        target_compile_options("${target}" PRIVATE
            $<$<CONFIG:Release>:-g>)
    endif()
endfunction()

function(att_target_enable_asan target)
    if (MSVC)
        target_compile_options(${target} PRIVATE
            /fsanitize=address
        )
        target_compile_definitions(${target} PRIVATE
            _DISABLE_VECTOR_ANNOTATION
        )
    else()
        target_compile_options(${target} PRIVATE
            -fsanitize=address
            -fsanitize=leak
            -fsanitize=undefined
        )
        target_link_options(${target} PRIVATE
            -fsanitize=address
            -fsanitize=leak
            -fsanitize=undefined
        )
    endif()
endfunction()

function(att_read_version_file output_var file_path)
    file(READ "${file_path}" version_text)
    string(REGEX REPLACE "[ \t\r\n]" "" version_text "${version_text}")
    if (NOT (version_text MATCHES "^[0-9](\\.[0-9])?(\\.[0-9])?(\\.[0-9])?$"))
        message(FATAL_ERROR "${file_path} invalid semantic version: \"${version_text}\"")
    endif()
    set(${output_var} "${version_text}" PARENT_SCOPE)
endfunction()

# clone a git submodule at a path relative to CMAKE_CURRENT_SOURCE_DIR
function(att_clone_submodule module_dir)
    find_program(GIT_CMD git DOC "Clone submodules." REQUIRED)
    set(abs_module_dir "${CMAKE_CURRENT_SOURCE_DIR}/${module_dir}")
    if (NOT EXISTS "${abs_module_dir}")
        message(FATAL_ERROR "Submodule ${abs_module_dir} not found.")
    endif()
    message(STATUS "Cloning submodule '${module_dir}'")
    execute_process(
        COMMAND "${GIT_CMD}" submodule --quiet update --init --recursive "${abs_module_dir}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    if (NOT EXISTS "${abs_module_dir}/.git")
        message(FATAL_ERROR "Submodule ${abs_module_dir} failed to clone.")
    endif()
endfunction()
