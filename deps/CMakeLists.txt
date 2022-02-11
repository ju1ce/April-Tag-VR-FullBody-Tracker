include(ExternalProject)
include(CMakePackageConfigHelpers)
include("${CMAKE_SOURCE_DIR}/CMake/platform.cmake")

find_program(GIT_CMD git REQUIRED)
find_program(CURL_CMD curl REQUIRED)

execute_process(COMMAND ${GIT_CMD} submodule update --init -j 2 
    "deps/apriltag" "deps/opencv" "deps/opencv_contrib" "deps/openvr"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

set(DEPS_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/install")
set(DEPS_INSTALL_DIR ${DEPS_INSTALL_DIR} PARENT_SCOPE)

set(DEPS_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")

# ---- Helpers ---- #
if (ON)

    set(setup_dirs_result)
    macro(setup_dirs libname)
        set(libdir "${CMAKE_CURRENT_SOURCE_DIR}/${libname}")
        set(setup_dirs_result
            PREFIX "${libdir}" SOURCE_DIR "${libdir}" DOWNLOAD_DIR "${libdir}"
            TMP_DIR "${DEPS_BINARY_DIR}/cmake-ep/tmp" 
            STAMP_DIR "${DEPS_BINARY_DIR}/cmake-ep/"
            INSTALL_DIR "${DEPS_INSTALL_DIR}")
        unset(libdir)
    endmacro()

endif()

# ---- Apriltag ---- #
if (ON)

    set(AT_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/apriltag")
    setup_dirs("apriltag")
    ExternalProject_Add(
        apriltag
        # Directories
        ${setup_dirs_result}
        BINARY_DIR "${DEPS_BINARY_DIR}/apriltag"
        # Disable download
        DOWNLOAD_COMMAND ""
        UPDATE_COMMAND ""
        # CMake handles the rest with default build and install
        CMAKE_ARGS -DBUILD_SHARED_LIBS=0 -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        # Other options
        EXCLUDE_FROM_ALL ON
    )

endif()

# ---- wxWidgets ---- #
if (ON)

    set(WX_URL_BASE "https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.5/")
    if (${WIN32})
        set(WX_SOURCE_ZIP "${WX_URL_BASE}/wxWidgets-3.1.5.7z")
    else()
        set(WX_SOURCE_ZIP "${WX_URL_BASE}/wxWidgets-3.1.5.tar.bz2")
    endif()

    setup_dirs("build/wxWidgets-src")
    ExternalProject_Add(
        wxWidgets
        # Directories
        ${setup_dirs_result}
        BINARY_DIR "${DEPS_BINARY_DIR}/wxWidgets"
        # Init the submodule
        DOWNLOAD_COMMAND ${CURL_CMD} -Ls -o "wx_source.7z" "${WX_SOURCE_ZIP}"
        COMMAND ${CMAKE_COMMAND} -E tar x "wx_source.7z"
        UPDATE_COMMAND ""
        # CMake handles the rest with default build and install
        CMAKE_ARGS -DwxBUILD_SHARED:BOOL=ON -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        # Other options
        EXCLUDE_FROM_ALL ON)

endif()

# ---- OpenVR ---- #
if (ON)
    set(OVR_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/openvr")

    set(OVR_INCLUDE_DIR "${DEPS_INSTALL_DIR}/include")
    set(OVR_LIB_OUTPUT "${DEPS_INSTALL_DIR}/bin/openvr_api${PLATFORM_SHARED_EXT}")
    set(OVR_IMPLIB_OUTPUT "${DEPS_INSTALL_DIR}/lib/openvr_api${PLATFORM_INTERFACE_EXT}")

    configure_package_config_file(
        "${CMAKE_SOURCE_DIR}/CMake/openvrConfig.cmake.in" 
        "${DEPS_BINARY_DIR}/openvrConfig.cmake.out"
        PATH_VARS OVR_LIB_OUTPUT OVR_IMPLIB_OUTPUT OVR_INCLUDE_DIR
        INSTALL_DESTINATION "${DEPS_INSTALL_DIR}"
        INSTALL_PREFIX "${DEPS_INSTALL_DIR}")

    set(OVR_LIB_PATH "${PLATFORM_OS1}${PLATFORM_ARCH}/${PLATFORM_LIB_PREFIX}openvr_api")

    add_custom_target(openvr
        COMMAND ${CMAKE_COMMAND} -E copy
            "${OVR_PREFIX}/headers/openvr.h" "${OVR_PREFIX}/headers/openvr_driver.h"
            "${OVR_PREFIX}/headers/openvr_api.json" "${OVR_INCLUDE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${OVR_PREFIX}/bin/${OVR_LIB_PATH}${PLATFORM_SHARED_EXT}"
            "${OVR_LIB_OUTPUT}"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${OVR_PREFIX}/lib/${OVR_LIB_PATH}${PLATFORM_INTERFACE_EXT}"
            "${OVR_IMPLIB_OUTPUT}"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${DEPS_BINARY_DIR}/openvrConfig.cmake.out"
            "${DEPS_INSTALL_DIR}/openvrConfig.cmake")

    clean_platform()
endif()

# ---- OpenCV Contrib ---- #
if (ON)

    set(CV_C_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/opencv_contrib")
    set(CV_C_BINARY_DIR "${DEPS_BINARY_DIR}/opencv_contrib")
    add_custom_target(opencv_contrib
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CV_C_PREFIX}/modules/aruco" "${CV_C_BINARY_DIR}/modules/aruco"
        VERBATIM)

endif()

# ---- OpenCV ---- #
if (ON)

    set(CV_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/opencv")
    setup_dirs("opencv")
    ExternalProject_Add(
        opencv
        # Directories
        ${setup_dirs_result}
        BINARY_DIR "${DEPS_BINARY_DIR}/opencv"
        # Disable download
        DOWNLOAD_COMMAND ""
        UPDATE_COMMAND ""
        # CMake handles the rest
        CMAKE_ARGS -DBUILD_JAVA:BOOL=OFF -DBUILD_PERF_TESTS:BOOL=OFF -DBUILD_TESTS:BOOL=OFF
            -DBUILD_opencv_apps:BOOL=OFF -DINSTALL_CREATE_DISTRIB:BOOL=ON
            -DVIDEOIO_PLUGIN_LIST:STRING=ffmpeg
            -DOPENCV_EXTRA_MODULES_PATH:PATH=${CV_C_BINARY_DIR}/modules
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        # Other options
        EXCLUDE_FROM_ALL ON
        DEPENDS opencv_contrib
    )

endif()
