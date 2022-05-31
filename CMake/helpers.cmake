include(ExternalProject)
include(CMakePackageConfigHelpers)

# This file depends on many of these shared options
include("${CUSTOM_CMAKE_FILES_DIR}/shared.cmake")

# Prefix every global define with ATT/att
# Use functions instead of macros when using local variables to not polute the global scope

# Wrapper for configure_package_config_file() with some default settings
# Adds the template file as a SOURCE property to <project_name>-install target
function(att_configure_package_config project_name)
    cmake_parse_arguments(_arg "" "" "PATH_VARS" ${ARGN})
    configure_package_config_file(
        "${CUSTOM_CMAKE_FILES_DIR}/${project_name}Config.cmake.in"
        "${DEPS_INSTALL_DIR}/${project_name}/${project_name}Config.cmake"
        PATH_VARS ${_arg_PATH_VARS}
        INSTALL_DESTINATION "${DEPS_INSTALL_DIR}/${project_name}"
        INSTALL_PREFIX "${DEPS_INSTALL_DIR}/${project_name}")
    set_property(TARGET ${project_name}-install APPEND PROPERTY SOURCES
        "${CUSTOM_CMAKE_FILES_DIR}/${project_name}Config.cmake.in")
endfunction()

# Wrapper for ExternalProject_Add() with some default settings, directory layout
# EXTRA_CMAKE_ARGS <arg...> Forwarded to CMAKE_ARGS, after some options like install dir, config, build shared, toolchain.
# EXTRA_EP_ARGS <arg...> Forwarded to ExternalProject_Add.
# BUILD_COMMAND <arg...> Forwarded to BUILD_COMMAND, has default if not provided.
function(att_add_external_project project_name)
    cmake_parse_arguments(_arg "" ""
        "EXTRA_CMAKE_ARGS;EXTRA_EP_ARGS;BUILD_COMMAND" ${ARGN})

    if(ATT_IS_MULTI_CONFIG)
        if (CMAKE_GENERATOR MATCHES "Visual Studio")
            set(multi_config_flag "--config" "$(Configuration)")
        else()
            set(multi_config_flag "--config" "$<CONFIG>")
        endif()
        unset(single_config_flag)
    else()
        set(single_config_flag "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
        unset(multi_config_flag)
    endif()

    if(DEFINED _arg_BUILD_COMMAND)
        set(build_cmd_default ${_arg_BUILD_COMMAND})
    else()
        set(build_cmd_default "${CMAKE_COMMAND}" --build <BINARY_DIR> --target install ${multi_config_flag})
    endif()

    if(ATT_TOOLCHAIN_FILE)
        set(toolchain_file_flag "-DCMAKE_TOOLCHAIN_FILE=${ATT_TOOLCHAIN_FILE}")
    else()
        unset(toolchain_file_flag)
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
        # because this gets applied to deps aswell, use > escaped to late evaluate the generator expression in the external project itself
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug$<ANGLE-R>:Debug$<ANGLE-R>$<$<BOOL:${BUILD_SHARED_LIBS}>:DLL>
        ${toolchain_file_flag}
        ${single_config_flag}
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
        -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW # enable CMAKE_MSVC_RUNTIME_LIBRARY
        ${_arg_EXTRA_CMAKE_ARGS}

        EXTRA_EP_ARGS
        INSTALL_DIR "${DEPS_INSTALL_DIR}/${project_name}"
        DEPENDS ${_arg_DEPENDS})
    att_add_install_compile_commands_step(${project_name} "<INSTALL_DIR>")
endfunction()

# Add one of our own projects, assumes <project_name> in current dir
# Sets DEPS_INSTALL_DIR and CUSTOM_CMAKE_FILES_DIR
# Creates a config stamp to prevent building configs that weren't setup
function(att_add_project project_name)
    cmake_parse_arguments(_arg "" "" "DEPENDS;EXTRA_CMAKE_ARGS" ${ARGN})
    att_add_external_project(
        ${project_name}
        EXTRA_CMAKE_ARGS
        "-DDEPS_INSTALL_DIR=${DEPS_INSTALL_DIR}"
        "-DCUSTOM_CMAKE_FILES_DIR=${CUSTOM_CMAKE_FILES_DIR}"
        ${_arg_EXTRA_CMAKE_ARGS}

        EXTRA_EP_ARGS
        INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
        BUILD_ALWAYS ON
        DEPENDS ${_arg_DEPENDS})
    att_ep_create_config_stamp(${project_name})
endfunction()

# Create a target to copy files
# args after named should be one or more of this layout ( INSTALL_ENTRY <dest_dir> <list of input files> )
# which will then install each entries list to
# dest_dir will be appended to install_prefix, and the input list will be placed at the top level of this folder
# input files must already exist at configure time
function(att_add_files_installer target_name install_prefix)
    set(input_files) # merged all entries input files
    set(output_files) # merged the created output file
    set(commands) # list of commands used by add_custom_command

    set(dest_dir) # current destination dir the entry is using, relative to install_prefix

    set(sep "INSTALL_ENTRY") # arg list separator

    foreach(elem ${ARGN})
        # Expect next elem to be the destination dir
        if(elem STREQUAL sep)
            # separator followed by separator
            if(dest_dir STREQUAL sep)
                message(FATAL_ERROR "Empty install entry")
            endif()

            # set next loop to expect a directory
            set(dest_dir ${sep})
        elseif(dest_dir STREQUAL sep)
            # this entries destination
            set(dest_dir "${elem}")
            set(dest_dir_path "${install_prefix}/${dest_dir}")
            list(APPEND output_dirs "${dest_dir_path}")
            list(APPEND commands COMMAND "${CMAKE_COMMAND}" -E make_directory "${dest_dir_path}")
        else()
            # we should be in this entries list of input files
            list(APPEND input_files "${elem}")
            get_filename_component(input_file_name "${elem}" NAME)
            set(output_file_path "${install_prefix}/${dest_dir}/${input_file_name}")
            list(APPEND output_files "${output_file_path}")
            list(APPEND commands COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${elem}" "${output_file_path}")
        endif()

        # dest dir should be INSTALL_ENTRY or a directory by now
        if(NOT dest_dir)
            message(FATAL_ERROR "Separate each destination dir with INSTALL_ENTRY")
        endif()
    endforeach()

    # Last arg was separator
    if(dest_dir STREQUAL sep)
        message(FATAL_ERROR "Empty install entry.")
    endif()

    add_custom_command(OUTPUT ${output_files} DEPENDS ${input_files} ${commands} VERBATIM)
    add_custom_target(${target_name} DEPENDS ${output_files})
endfunction()

# Creates the file config-$<CONFIG> in ExternalProjectFiles/stamp when the superproject builds that config
function(att_ep_create_config_stamp project_name)
    # Create a config stamp to allow subprojects to test which configuration they are allowed to build
    set(CONFIG_STAMP_FILE "<BINARY_DIR>/ExternalProjectFiles/stamp/config-$<LOWER_CASE:$<CONFIG>>")
    ExternalProject_Add_Step(${project_name} create-config-stamp
        DEPENDERS build BYPRODUCTS "${CONFIG_STAMP_FILE}"
        COMMAND "${CMAKE_COMMAND}" -E touch "${CONFIG_STAMP_FILE}")
endfunction()

# Checks whether the superproject has generated the requested config yet, error if not
function(att_check_config_stamp target_name)
    add_custom_command(TARGET ${target_name} PRE_BUILD
        COMMENT "If this fails, build the config with the superproject first."
        COMMAND ${CMAKE_COMMAND} -E cat
        "${CMAKE_CURRENT_BINARY_DIR}/ExternalProjectFiles/stamp/config-$<LOWER_CASE:$<CONFIG>>")
endfunction()

# Find a <package_name>Config.cmake in DEPS_INSTALL_DIR
macro(att_find_dep package_name)
    find_package(${package_name} ${ARGN} PATHS "${DEPS_INSTALL_DIR}" NO_DEFAULT_PATH)
endmacro()
