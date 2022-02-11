# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.5)
project("AprilTagTrackers")

set(SEARCH_LOCAL_DEPS PATHS "${DEPS_INSTALL_DIR}" NO_DEFAULT_PATH)
# Find prebuilt libraries
find_package(OpenCV REQUIRED ${SEARCH_LOCAL_DEPS})
find_package(apriltag REQUIRED ${SEARCH_LOCAL_DEPS})
find_package(openvr REQUIRED ${SEARCH_LOCAL_DEPS})

set(wxWidgets_ROOT_DIR "${DEPS_INSTALL_DIR}")
find_package(wxWidgets REQUIRED core base net)
include(${wxWidgets_USE_FILE})

# Add source to this project's executable.
file(GLOB ATT_SOURCE "*.cpp" "*.h" "IPC/*.cpp" "IPC/*.h")
add_executable (${PROJECT_NAME} ${ATT_SOURCE})
# Set the root of includes, rather than relative
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Link libraries defined as targets, includes are automatic
target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${OpenCV_LIBS} ${wxWidgets_LIBRARIES} openvr::openvr apriltag::apriltag)

if (MSVC)
    # tell the msvc linker this is a windows application
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    # useful msvc flags
    target_compile_options(${PROJECT_NAME} PUBLIC "/MP" "/ZI")
    target_link_options(${PROJECT_NAME} PUBLIC "/INCREMENTAL")
endif()

# Install target to bin folder
install(TARGETS ${PROJECT_NAME} RUNTIME)
