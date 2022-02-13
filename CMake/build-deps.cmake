function(build_deps config)

if (${DEPS_BUILD_FAILED})
    return()
endif()

message(STATUS "Building deps: ${config}")

if (DEFINED CMAKE_CONFIGURATION_TYPES)
    set(DEPS_BUILD_MULTI --config ${config})
else()
    set(DEPS_BUILD_SINGLE -DCMAKE_BUILD_TYPE:STRING=${config})
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -B "${DEPS_BINARY_DIR}" -S "${DEPS_PREFIX}"
        ${DEPS_BUILD_SINGLE}
        -DCMAKE_INSTALL_PREFIX:PATH=${DEPS_INSTALL_DIR}
        -DBUILD_SHARED:BOOL=${BUILD_DEPS_SHARED}
    RESULT_VARIABLE DEPS_BUILD_RESULT)

if (NOT ${DEPS_BUILD_RESULT} EQUAL 0)
    set(DEPS_BUILD_FAILED TRUE PARENT_SCOPE)
    return()
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${DEPS_BINARY_DIR}" ${DEPS_BUILD_MULTI} --target AprilTagTracker
    RESULT_VARIABLE DEPS_BUILD_RESULT)

if (NOT ${DEPS_BUILD_RESULT} EQUAL 0)
    set(DEPS_BUILD_FAILED TRUE PARENT_SCOPE)
    return()
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} --install "${DEPS_BINARY_DIR}" ${DEPS_BUILD_MULTI} --prefix "${DEPS_INSTALL_DIR}"
    RESULT_VARIABLE DEPS_BUILD_RESULT)

if (NOT ${DEPS_BUILD_RESULT} EQUAL 0)
    set(DEPS_BUILD_FAILED TRUE PARENT_SCOPE)
endif()

endfunction()
