# CMakeList.txt : CMake project for AprilTagTrackers, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.5)
project("AprilTagTrackers")

set(SEARCH_LOCAL PATHS "${DEPS_INSTALL_DIR}" NO_DEFAULT_PATH)
# Find prebuilt libraries
find_package(OpenCV REQUIRED ${SEARCH_LOCAL})
find_package(apriltag REQUIRED ${SEARCH_LOCAL})
find_package(openvr REQUIRED ${SEARCH_LOCAL})
find_package(wxWidgets REQUIRED mono ${SEARCH_LOCAL})

# Add source to this project's executable.
file(GLOB ATT_SOURCE "*.cpp" "*.h" "IPC/*.cpp" "IPC/*.h")
add_executable (${PROJECT_NAME} ${ATT_SOURCE})
# Set the root of includes, rather than relative
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Link libraries defined as targets, includes are automatic
target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${OpenCV_LIBS} ${wxWidgets_LIBRARIES} openvr::openvr apriltag::apriltag)
target_include_directories(${PROJECT_NAME} PUBLIC ${wxWidgets_INCLUDE_DIRS})

if (MSVC)
    # tell the msvc linker this is a windows application
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    # useful msvc flags
    # target_compile_options(${PROJECT_NAME} PUBLIC "/MP" "/ZI")
    # target_link_options(${PROJECT_NAME} PUBLIC "/INCREMENTAL")
endif()

# Install target to bin folder
install(TARGETS ${PROJECT_NAME} RUNTIME)
install(FILES $<TARGET_FILE:openvr::openvr> DESTINATION "bin")