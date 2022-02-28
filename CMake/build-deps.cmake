function(build_deps config)

if (NOT ${BUILD_DEPS})
    return()
endif()

if ((NOT ${REBUILD_DEPS}) AND BUILD_DEPS_SUCCESS_${config})
    message(STATUS "Deps already built for ${config} - skipping.")
    return()
endif()

set(LOG_TO_FILE)
if (NOT ${BUILD_DEPS_VERBOSE})
    set(LOG_TO_FILE OUTPUT_FILE "${DEPS_BINARY_DIR}/build-deps.cmake.log")
endif()

message(STATUS "Build deps: ${config}")

if (DEFINED CMAKE_CONFIGURATION_TYPES)
    set(DEPS_BUILD_MULTI --config ${config})
else()
    set(DEPS_BUILD_SINGLE -DCMAKE_BUILD_TYPE:STRING=${config})
endif()

make_directory("${DEPS_BINARY_DIR}")
make_directory("${DEPS_INSTALL_DIR}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -B "${DEPS_BINARY_DIR}" -S "${DEPS_PREFIX}" -G "${CMAKE_GENERATOR}"
        ${DEPS_BUILD_SINGLE}
        -DCMAKE_INSTALL_PREFIX:PATH=${DEPS_INSTALL_DIR}
        -DBUILD_SHARED:BOOL=${BUILD_DEPS_SHARED}
    ${LOG_TO_FILE}
    RESULT_VARIABLE DEPS_BUILD_RESULT)

if (NOT ${DEPS_BUILD_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to configure deps, cmake log saved to deps/build/build-deps.cmake.log \nError: ${DEPS_BUILD_RESULT}")
endif()
message(STATUS "Configure done.")

execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${DEPS_BINARY_DIR}" ${DEPS_BUILD_MULTI} --target AprilTagTrackers
    ${LOG_TO_FILE}
    RESULT_VARIABLE DEPS_BUILD_RESULT)

if (NOT ${DEPS_BUILD_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to build deps, cmake log saved to deps/build/build-deps.cmake.log \nError: ${DEPS_BUILD_RESULT}")
endif()
message(STATUS "Build done.")

execute_process(
    COMMAND ${CMAKE_COMMAND} --install "${DEPS_BINARY_DIR}" ${DEPS_BUILD_MULTI} --prefix "${DEPS_INSTALL_DIR}"
    ${LOG_TO_FILE}
    RESULT_VARIABLE DEPS_BUILD_RESULT)

if (NOT ${DEPS_BUILD_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to install deps, cmake log saved to deps/build/build-deps.cmake.log \nError: ${DEPS_BUILD_RESULT}")
endif()
message(STATUS "Install done.")

set(BUILD_DEPS_SUCCESS_${config} ON CACHE INTERNAL "Build deps success.")

endfunction()