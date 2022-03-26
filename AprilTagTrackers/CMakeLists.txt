# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.11)
project("AprilTagTrackers" CXX)

option(USE_ASAN "Create an address sanitizer build." OFF)
option(BUILD_SHARED_LIBS "Attempt to link libraries as shared or static." OFF)
option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compile commands for language servers." OFF)

set(DEPS_INSTALL_DIR "deps/install" CACHE PATH "Deps install directory.")

# If using a multi config generator, wxWidgets and opencv due to this random blog post
# remove the other build types and msbuild errors if we dont do the same.
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "Limited configuration types to fix dependencies." FORCE)
endif()

# Find built libraries
set(FIND_LOCAL PATHS "${DEPS_INSTALL_DIR}" NO_DEFAULT_PATH)
find_package(OpenCV REQUIRED ${FIND_LOCAL})
find_package(apriltag REQUIRED ${FIND_LOCAL})
find_package(openvr REQUIRED ${FIND_LOCAL})
find_package(wxWidgets REQUIRED core base ${FIND_LOCAL})

# Add source to this project's executable.
file(GLOB_RECURSE ATT_SOURCE "*.cpp" "*.h")
add_executable(${PROJECT_NAME} ${ATT_SOURCE})

# Set the root of includes, rather than relative
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

set(ENV{GCC_COLORS} "error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01")
target_compile_options(${PROJECT_NAME} PRIVATE -fcolor-diagnostics /clang:-fdiagnostics-show-template-tree)
add_compile_options(-fcolor-diagnostics /clang:-fcolor-diagnostics)

# Link libraries defined as targets, includes are automatic
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${OpenCV_LIBS} wx::mono openvr::openvr apriltag::apriltag)

# Install target to bin folder
install(TARGETS ${PROJECT_NAME} RUNTIME)
install(FILES $<TARGET_FILE:openvr::openvr> DESTINATION "bin")

# Ensure compiler with c++17 language features
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Enable LTO in release
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# Set cross-platform defines in sources
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "OS_WIN=1")

    # Link as gui app instead of console app
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "OS_LINUX=1")
else()
    message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}, compilation will fail.")
endif()

# Build with Address Sanitizer
if(USE_ASAN)
    unset(ASAN_FLAGS)

    if(MSVC)
        set(ASAN_FLAGS /fsanitize=address)
    else()
        set(ASAN_FLAGS
            -fsanitive=address
            -fsanitive=leak
            -fsanitive=undefined)
    endif()

    target_compile_options(${PROJECT_NAME} PRIVATE ${ASAN_FLAGS})
endif()
