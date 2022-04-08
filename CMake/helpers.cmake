include(ExternalProject)
include(CMakePackageConfigHelpers)

get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(IS_MULTI_CONFIG)
    # If using a multi config generator, wxWidgets and opencv for some reason
    # remove the other build types and msbuild errors if we dont do the same.
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "Limited configuration types to fix dependencies." FORCE)

    # cmake can only create compile commands with single config generators
    # TODO: Try to generate our own for Visual Studio?
    set(EXPORT_COMPILE_COMMANDS OFF)

    # Only expands on multi config, single config generators are fine with default commands
    set(MULTI_CONFIG_FLAG "--config" "$(Configuration)")
    unset(SINGLE_CONFIG_FLAG)
else()
    # Set our default build type to release
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Debug or Release.")

    # Only expands on single config, not really needed, but multiconfig warns when its unused
    set(SINGLE_CONFIG_FLAG "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
    unset(MULTI_CONFIG_FLAG)
endif()

# Toolchain path needs to be absolute for external projects
if(CMAKE_TOOLCHAIN_FILE)
    get_filename_component(toolchain_file_path "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)
    set(TOOLCHAIN_FLAG "-DCMAKE_TOOLCHAIN_FILE=${toolchain_file_path}")
endif()

# Wrapper for configure_package_config_file() with some default settings
# Adds the template file as a SOURCE property to <project_name>-install target
function(att_configure_package_config project_name)
    cmake_parse_arguments(_arg "" "" "PATH_VARS" ${ARGN})
    configure_package_config_file(
        "${CUSTOM_CMAKE_FILES}/${project_name}Config.cmake.in"
        "${DEPS_INSTALL_DIR}/${project_name}/${project_name}Config.cmake"
        PATH_VARS ${_arg_PATH_VARS}
        INSTALL_DESTINATION "${DEPS_INSTALL_DIR}/${project_name}"
        INSTALL_PREFIX "${DEPS_INSTALL_DIR}/${project_name}")
    set_property(TARGET ${project_name}-install APPEND PROPERTY SOURCES
        "${CUSTOM_CMAKE_FILES}/${project_name}Config.cmake.in")
endfunction()

# Wrapper for ExternalProject_Add() with some default settings, directory layout
# EXTRA_CMAKE_ARGS <arg...> Forwarded to CMAKE_ARGS, after some options like install dir, config, build shared, toolchain.
# EXTRA_EP_ARGS <arg...> Forwarded to ExternalProject_Add.
# BUILD_COMMAND <arg...> Forwarded to BUILD_COMMAND, has default if not provided.
function(att_add_external_project project_name)
    cmake_parse_arguments(_arg "" ""
        "EXTRA_CMAKE_ARGS;EXTRA_EP_ARGS;BUILD_COMMAND" ${ARGN})

    if(DEFINED _arg_BUILD_COMMAND)
        set(build_cmd_default ${_arg_BUILD_COMMAND})
    else()
        set(build_cmd_default "${CMAKE_COMMAND}" --build <BINARY_DIR> --target install ${MULTI_CONFIG_FLAG})
    endif()

    set(project_source "${CMAKE_CURRENT_SOURCE_DIR}/${project_name}")
    set(project_binary "${CMAKE_CURRENT_BINARY_DIR}/${project_name}")
    set(epw_files_dir "${CMAKE_CURRENT_BINARY_DIR}/${project_name}/ExternalProjectFiles")

    ExternalProject_Add(
        ${project_name}
        PREFIX "${project_source}"
        SOURCE_DIR "${project_source}"
        DOWNLOAD_DIR "${project_source}"
        BINARY_DIR "${project_binary}"
        TMP_DIR "${epw_files_dir}/tmp"
        STAMP_DIR "${epw_files_dir}/stamp"
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DBUILD_SHARED_LIBS=$<BOOL:${BUILD_SHARED_LIBS}>
        -DCMAKE_EXPORT_COMPILE_COMMANDS=${EXPORT_COMPILE_COMMANDS}
        -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_LIBS}>:DLL>
        ${TOOLCHAIN_FLAG}
        ${SINGLE_CONFIG_FLAG}
        ${_arg_EXTRA_CMAKE_ARGS}

        UPDATE_COMMAND ""
        BUILD_COMMAND ${build_cmd_default}
        INSTALL_COMMAND ""
        STEP_TARGETS install
        ${_arg_EXTRA_EP_ARGS})
endfunction()

# Adds external project step to copy compile_commands.json to install_dir
function(att_add_install_compile_commands_step target_name install_dir)
    if(EXPORT_COMPILE_COMMANDS)
        ExternalProject_Add_Step(${target_name} install-compile-commands
            DEPENDERS install DEPENDEES configure
            COMMAND ${CMAKE_COMMAND} -E make_directory "${install_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy "<BINARY_DIR>/compile_commands.json" "${install_dir}/compile_commands.json")
    endif()
endfunction()

# Add a dependency deps/<project_name> and installs to <DEPS_INSTALL_DIR>/<project_name>
# Forwards DEPENDS arg, disables developer warnings
# Adds compile commands install step to copy to install dir
function(att_add_dep project_name)
    cmake_parse_arguments(_arg "" "" "DEPENDS;EXTRA_CMAKE_ARGS" ${ARGN})
    att_add_external_project(
        ${project_name}
        EXTRA_CMAKE_ARGS -Wno-dev
        ${_arg_EXTRA_CMAKE_ARGS}

        EXTRA_EP_ARGS
        INSTALL_DIR "${DEPS_INSTALL_DIR}/${project_name}"
        DEPENDS ${_arg_DEPENDS})
    att_add_install_compile_commands_step(${project_name} "<INSTALL_DIR>")
endfunction()

function(att_replace_lang_flags match_value replace_value)
    foreach(lang_val ${ARGN})
        foreach(suffix_val "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
            set(flags_var "CMAKE_${lang_val}_FLAGS${suffix_val}")
            string(REPLACE ${match_value} ${replace_value} ${flags_var} "${${flags_var}}")
        endforeach()
    endforeach()
endfunction()
