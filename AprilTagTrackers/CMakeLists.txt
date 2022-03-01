# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.1)
project("AprilTagTrackers" CXX)
set(CMAKE_CXX_STANDARD 14)

set(SEARCH_LOCAL PATHS "${DEPS_INSTALL_DIR}" NO_DEFAULT_PATH)
# Find prebuilt libraries
find_package(OpenCV REQUIRED ${SEARCH_LOCAL})
find_package(apriltag REQUIRED ${SEARCH_LOCAL})
find_package(openvr REQUIRED ${SEARCH_LOCAL})
find_package(wxWidgets REQUIRED mono ${SEARCH_LOCAL})

set(MAKE_WIN_APP)
if (WIN32)
    set(MAKE_WIN_APP "WIN32")
endif()

# Add source to this project's executable.
file(GLOB_RECURSE ATT_SOURCE "*.cpp" "*.h")
add_executable(${PROJECT_NAME} ${MAKE_WIN_APP} ${ATT_SOURCE})
# Set the root of includes, rather than relative
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Link libraries defined as targets, includes are automatic
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${OpenCV_LIBS} wx::mono openvr::openvr apriltag::apriltag)

if (${USE_PRECOMP} AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(${PROJECT_NAME} PRIVATE "precomp.h")
endif()

# Setup platform defines
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "OS_WIN")
elseif (UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "OS_LINUX")
else()
    message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}, compilation will fail.")
endif()

# Setup compiler specific options
if (${MSVC})
    target_compile_options(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Release>:/GL;/GA;/Gw>")
    target_link_options(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Release>:/LTCG;/OPT:ICF=3>")
endif()

if (${ASAN})
    target_compile_options(${PROJECT_NAME} PRIVATE
        "fsanitize=address")
endif()

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Install target to bin folder
install(TARGETS ${PROJECT_NAME} RUNTIME)
install(FILES $<TARGET_FILE:openvr::openvr> DESTINATION "bin")