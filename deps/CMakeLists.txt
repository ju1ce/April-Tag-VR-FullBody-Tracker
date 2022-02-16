cmake_minimum_required(VERSION 3.5)
project("deps")

set(BUILD_SHARED OFF CACHE BOOL "Build shared libs.")

add_custom_target(AprilTagTrackers)

# ---- Helpers ---- #
if (ON)

    set(CUSTOM_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
    set(BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set(INT_DIR "${CMAKE_INSTALL_PREFIX}")
    set_property(DIRECTORY PROPERTY EP_STEP_TARGETS install)

    include(ExternalProject)
    include(CMakePackageConfigHelpers)
    include("${CUSTOM_SCRIPTS}/platform.cmake")

    find_program(GIT_CMD git REQUIRED)
    find_program(CURL_CMD curl REQUIRED)

    set(SUBMODULE_UPDATE_CMD ${GIT_CMD} submodule update --depth 1 -f --checkout --init)

    set(setup_dirs_result)
    macro(setup_dirs namespace libname srcdir)
        set(${namespace}_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/${srcdir}")
        set(${namespace}_BIN_DIR "${BIN_DIR}/${libname}")

        set(dir "${CMAKE_CURRENT_SOURCE_DIR}/${srcdir}")
        set(setup_dirs_result
            PREFIX "${dir}" SOURCE_DIR "${dir}" DOWNLOAD_DIR "${dir}"
            BINARY_DIR "${BIN_DIR}/${libname}"
            TMP_DIR "${BIN_DIR}/${libname}/ExternalProject"
            STAMP_DIR "${BIN_DIR}/${libname}/ExternalProject"
            INSTALL_DIR "${INT_DIR}")
        unset(dir)
    endmacro()

    function(configure_install_file _c_NAME)
        cmake_parse_arguments(_c "" "" "PATH_VARS" ${ARGN})
        configure_package_config_file(
            "${CUSTOM_SCRIPTS}/${_c_NAME}.cmake.in"
            "${INT_DIR}/${_c_NAME}.cmake"
            PATH_VARS ${_c_PATH_VARS}
            INSTALL_DESTINATION "${INT_DIR}"
            INSTALL_PREFIX "${INT_DIR}")
    endfunction()

    if (DEFINED CMAKE_CONFIGURATION_TYPES)
        set(DEPS_BUILD_MULTI BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>)
    else()
        set(DEPS_BUILD_SINGLE -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
    endif()

endif()

# ---- Apriltag ---- #
if (ON)

    setup_dirs(AT "apriltag" "apriltag")
    ExternalProject_Add(
        apriltag
        # Directories
        ${setup_dirs_result}
        # Disable download
        DOWNLOAD_COMMAND ${SUBMODULE_UPDATE_CMD} <SOURCE_DIR>
        UPDATE_COMMAND ""
        # CMake handles the rest with default build and install
        CMAKE_ARGS -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED}
            -DBUILD_EXAMPLES:BOOL=OFF -DBUILD_PYTHON_WRAPPER:BOOL=OFF
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            ${DEPS_BUILD_SINGLE}
        ${DEPS_BUILD_MULTI}
        EXCLUDE_FROM_ALL 1
    )
    add_dependencies(AprilTagTrackers apriltag-install)

endif()

# ---- wxWidgets ---- #
if (ON)

    setup_dirs(WX "wxWidgets" "build/wxWidgets-src")

    set(WX_URL_BASE "https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.5/")
    if (${WIN32})
        set(WX_SOURCE_ZIP "${WX_URL_BASE}/wxWidgets-3.1.5.7z")
    else()
        set(WX_SOURCE_ZIP "${WX_URL_BASE}/wxWidgets-3.1.5.tar.bz2")
    endif()

    ExternalProject_Add(
        wxWidgets
        # Directories
        ${setup_dirs_result}
        # Init the submodule
        DOWNLOAD_COMMAND ${CURL_CMD} -Ls -o "wx_source.7z" "${WX_SOURCE_ZIP}"
        COMMAND ${CMAKE_COMMAND} -E tar x "wx_source.7z"
        UPDATE_COMMAND ""
        # CMake handles the rest with default build and install
        CMAKE_ARGS -DwxBUILD_SHARED:BOOL=${BUILD_SHARED}
            -DwxBUILD_MONOLITHIC:BOOL=ON
            -DwxBUILD_OPTIMISE:BOOL=$<CONFIG:Release>
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            ${DEPS_BUILD_SINGLE}
        ${DEPS_BUILD_MULTI}
        EXCLUDE_FROM_ALL 1
    )
    add_dependencies(AprilTagTrackers wxWidgets-install)

    set(WX_ROOT_DIR "${INT_DIR}")
    configure_install_file("wxWidgetsConfig"
        PATH_VARS WX_ROOT_DIR)
    target_sources(AprilTagTrackers PRIVATE "${CUSTOM_SCRIPTS}/wxWidgetsConfig.cmake.in")

endif()

# ---- OpenVR ---- #
if (ON)

    setup_dirs(OVR "openvr" "openvr")

    set(OVR_INCLUDE_DIR "${INT_DIR}/include")
    set(OVR_LIB_OUTPUT "${INT_DIR}/bin/openvr_api${PLATFORM_SHARED_EXT}")
    set(OVR_IMPLIB_OUTPUT "${INT_DIR}/lib/openvr_api${PLATFORM_INTERFACE_EXT}")
    configure_install_file("openvrConfig"
        PATH_VARS OVR_INCLUDE_DIR OVR_LIB_OUTPUT OVR_IMPLIB_OUTPUT)

    make_directory("${INT_DIR}/include")
    make_directory("${INT_DIR}/lib")
    make_directory("${INT_DIR}/bin")

    set(OVR_LIB_PATH "${PLATFORM_OS1}${PLATFORM_ARCH}/${PLATFORM_LIB_PREFIX}openvr_api")
    add_custom_command(OUTPUT "${OVR_LIB_OUTPUT}"
        COMMAND ${SUBMODULE_UPDATE_CMD} ${OVR_PREFIX}
        COMMAND ${CMAKE_COMMAND} -E copy
            "${OVR_PREFIX}/headers/openvr.h" "${OVR_PREFIX}/headers/openvr_driver.h"
            "${OVR_PREFIX}/headers/openvr_api.json" "${OVR_INCLUDE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${OVR_PREFIX}/bin/${OVR_LIB_PATH}${PLATFORM_SHARED_EXT}"
            "${OVR_LIB_OUTPUT}"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${OVR_PREFIX}/lib/${OVR_LIB_PATH}${PLATFORM_INTERFACE_EXT}"
            "${OVR_IMPLIB_OUTPUT}"
        VERBATIM)
    add_custom_target(openvr DEPENDS "${OVR_LIB_OUTPUT}")
    add_dependencies(AprilTagTrackers openvr)
    target_sources(AprilTagTrackers PRIVATE "${CUSTOM_SCRIPTS}/openvrConfig.cmake.in")

endif()

# ---- OpenCV Contrib ---- #
if (ON)

    setup_dirs(CV_C "opencv_contrib" "opencv_contrib")
    add_custom_command(OUTPUT "${CV_C_BIN_DIR}/modules/aruco/CMakeLists.txt"
        COMMAND ${SUBMODULE_UPDATE_CMD} ${CV_C_PREFIX}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CV_C_PREFIX}/modules/aruco" "${CV_C_BIN_DIR}/modules/aruco"
        VERBATIM)
    add_custom_target(opencv_contrib DEPENDS "${CV_C_BIN_DIR}/modules/aruco/CMakeLists.txt")

endif()

# ---- OpenCV ---- #
if (ON)

    set(CV_BUILD_SMALL
        -DBUILD_opencv_apps:BOOL=OFF
        -DBUILD_JAVA:BOOL=OFF
        -DBUILD_opencv_python3:BOOL=OFF
        -DBUILD_opencv_python_bindings_generator:BOOL=OFF)

    setup_dirs(CV "opencv" "opencv")
    ExternalProject_Add(
        opencv
        # Directories
        ${setup_dirs_result}
        # Disable download
        DOWNLOAD_COMMAND ${SUBMODULE_UPDATE_CMD} <SOURCE_DIR>
        UPDATE_COMMAND ""
        # CMake handles the rest
        CMAKE_ARGS -DBUILD_PERF_TESTS:BOOL=OFF -DBUILD_TESTS:BOOL=OFF
            -DVIDEOIO_PLUGIN_LIST:STRING=ffmpeg
            -DOPENCV_EXTRA_MODULES_PATH:PATH=${CV_C_BIN_DIR}/modules
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=OFF
            -DBUILD_WITH_STATIC_CRT:BOOL=OFF
            -DENABLE_LTO:BOOL=$<CONFIG:Release>
            -DOPENCV_GENERATE_SETUPVARS:BOOL=OFF
            ${CV_BUILD_SMALL}
            ${DEPS_BUILD_SINGLE}
        ${DEPS_BUILD_MULTI}
        # Other options
        EXCLUDE_FROM_ALL 1
        DEPENDS opencv_contrib)
    add_dependencies(AprilTagTrackers opencv-install)

endif()
