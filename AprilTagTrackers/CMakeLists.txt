# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.5)
project("AprilTagTrackers" CXX)

option(USE_ASAN "Create an address sanitizer build." OFF)
option(BUILD_SHARED_LIBS "Attempt to link libraries as shared or static." OFF)
option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compile commands for language servers." OFF)
set(DEPS_INSTALL_DIR "${DEPS_PREFIX}/install" CACHE PATH "Deps install directory.")

# Don't look for globally installed packages
set(SEARCH_LOCAL PATHS "${DEPS_INSTALL_DIR}" NO_DEFAULT_PATH)
# Find built libraries
find_package(OpenCV REQUIRED ${SEARCH_LOCAL})
find_package(apriltag REQUIRED ${SEARCH_LOCAL})
find_package(openvr REQUIRED ${SEARCH_LOCAL})
find_package(wxWidgets REQUIRED mono ${SEARCH_LOCAL})

# Add source to this project's executable.
file(GLOB_RECURSE ATT_SOURCE "*.cpp" "*.h")
add_executable(${PROJECT_NAME} ${ATT_SOURCE})

# Set the root of includes, rather than relative
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Link libraries defined as targets, includes are automatic
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${OpenCV_LIBS} wx::mono openvr::openvr apriltag::apriltag)

# Install target to bin folder
install(TARGETS ${PROJECT_NAME} RUNTIME)
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> CONFIGURATIONS Debug DESTINATION "bin")
install(FILES $<TARGET_FILE:openvr::openvr> DESTINATION "bin")

# Ensure compiler with c++14 language features
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
# Enable LTO in release
set_target_properties(wx::mono PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# Setup platform options
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "OS_WIN=1")
    # Link as gui app instead of console app
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "OS_LINUX=1")
else()
    message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}, compilation will fail.")
endif()

# Setup compiler specific options
if(MSVC)
    # Include PDB within exe
    # target_compile_options(${PROJECT_NAME} PUBLIC
    #     "$<$<CONFIG:Debug>:/>")
endif()

# Build with Address Sanitizer
if(USE_ASAN)
    target_compile_options(${PROJECT_NAME} PRIVATE
        "fsanitize=address")

    if(NOT MSVC)
        # clang and gcc provide some more sanitizer abilities
        target_compile_options(${PROJECT_NAME} PRIVATE
            "fsanitize=leak"
            "fsanitize=undefined")
    endif()
endif()