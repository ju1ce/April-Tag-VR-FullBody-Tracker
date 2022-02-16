function(build_deps config)

message(STATUS "Build deps: ${config}")

if (DEFINED CMAKE_CONFIGURATION_TYPES)
    set(DEPS_BUILD_MULTI --config ${config})
else()
    set(DEPS_BUILD_SINGLE -DCMAKE_BUILD_TYPE:STRING=${config})
endif()

make_directory("${DEPS_BINARY_DIR}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -B "${DEPS_BINARY_DIR}" -S "${DEPS_PREFIX}" -G "${CMAKE_GENERATOR}"
        ${DEPS_BUILD_SINGLE}
        -DCMAKE_INSTALL_PREFIX:PATH=${DEPS_INSTALL_DIR}
        -DBUILD_SHARED:BOOL=${BUILD_DEPS_SHARED}
    OUTPUT_FILE "${DEPS_BINARY_DIR}/deps_build.log"
    RESULT_VARIABLE DEPS_BUILD_RESULT)

if (NOT ${DEPS_BUILD_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to configure deps: \nError: ${DEPS_BUILD_RESULT}")
endif()
message(STATUS "Configure done.")

execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${DEPS_BINARY_DIR}" ${DEPS_BUILD_MULTI} --target AprilTagTrackers
    OUTPUT_FILE "${DEPS_BINARY_DIR}/deps_build.log"
    RESULT_VARIABLE DEPS_BUILD_RESULT)

if (NOT ${DEPS_BUILD_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to build deps: \nError: ${DEPS_BUILD_RESULT}")
endif()
message(STATUS "Build done.")

execute_process(
    COMMAND ${CMAKE_COMMAND} --install "${DEPS_BINARY_DIR}" ${DEPS_BUILD_MULTI} --prefix "${DEPS_INSTALL_DIR}"
    OUTPUT_FILE "${DEPS_BINARY_DIR}/deps_build.log"
    RESULT_VARIABLE DEPS_BUILD_RESULT)

if (NOT ${DEPS_BUILD_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to install deps: \nError: ${DEPS_BUILD_RESULT}")
endif()
message(STATUS "Install done.")

endfunction()
