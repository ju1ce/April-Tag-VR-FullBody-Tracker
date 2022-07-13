# Shared defines and options between all projects

# Prefix every global define with ATT/att
# Use functions instead of macros when using local variables to not polute the global scope

# Fix default install path on windows
# Noone wants to provide admin access and install to C:/Program Files (x86)/
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "Install directory." FORCE)
    message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")
endif()

# Alias the is multi config property, useful for fixing visual studio quirks
get_property(ATT_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(NOT ATT_IS_MULTI_CONFIG)
    # Set default build type to release on single config
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Debug or Release.")
endif()

function(att_read_version_file output_var file_path)
    file(READ "${file_path}" version_text)
    string(REGEX REPLACE "[ \t\r\n]" "" version_text "${version_text}")
    if (NOT (version_text MATCHES "^[0-9](\\.[0-9])?(\\.[0-9])?(\\.[0-9])?$"))
        message(FATAL_ERROR "${file_path} invalid semantic version: \"${version_text}\"")
    endif()
    set(${output_var} "${version_text}" PARENT_SCOPE)
endfunction()
