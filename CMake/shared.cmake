# Shared defines and options between all projects

# Prefix every global define with ATT/att
# Use functions instead of macros when using local variables to not polute the global scope

# Useful variables that are also required by helper functions
# These will always get defined, but they won't conflict,

# Global build options, used by helpers, deps, and all our projects
option(ENABLE_LTO "Enable link time optimizations for release builds" ON)
option(BUILD_SHARED_LIBS "Attempt to link libraries as shared or static." OFF)
set(DEPS_INSTALL_DIR "" CACHE PATH "Deps install directory.")

macro(AprilTagTrackers_options)
    option(ENABLE_ASAN "Create an address sanitizer build." OFF)
    option(ENABLE_PS3EYE "Enable ps3eye camera support, Windows only for now." ${WIN32})
    option(ENABLE_ASSERT "Enable ATASSERT in release builds." OFF)
    option(ENABLE_OUTPUT_LOG_FILE "Redirect stdout and stderr to an output.log file." ON)
    set(LOG_LEVEL "" CACHE STRING "0 - Nothing, 1 - Fatal, 2 - Error, 3 - Trace")
endmacro()

# Fix default install path on windows
# Noone wants to provide admin access and install to C:/Program Files (x86)/
if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(FATAL_ERROR "On Windows the default install destination is ${CMAKE_INSTALL_PREFIX}, \
which requires admin rights and usually results in errors. \
Explicitly set with -DCMAKE_INSTALL_PREFIX= to a relative or absolute path. \
Rerun if you are sure about installing to this location.")
endif()

# Alias the is multi config property, useful for fixing visual studio quirks
get_property(ATT_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(ATT_IS_MULTI_CONFIG)
    # If using a multi config generator, wxWidgets and opencv for some reason
    # remove the other build types and msbuild errors if we dont do the same.
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "Choose Debug or Release builds. Limited configuration types to fix dependencies." FORCE)

    # cmake can only create compile commands with single config generators
    # TODO: Try to generate our own for Visual Studio?
    set(CMAKE_EXPORT_COMPILE_COMMANDS OFF CACHE BOOL "Generate compile_commands.json" FORCE)
else()
    # Set our default build type to release on single config
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Debug or Release.")
endif()

# Toolchain path needs to be absolute for external projects
if(CMAKE_TOOLCHAIN_FILE)
    get_filename_component(ATT_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)
else()
    unset(ATT_TOOLCHAIN_FILE)
endif()
