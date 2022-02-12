include(ExternalProject)
include(CMakePackageConfigHelpers)

find_program(GIT_CMD git REQUIRED)
find_program(CURL_CMD curl REQUIRED)

set(DEPS_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/install")
set(DEPS_INSTALL_DIR ${DEPS_INSTALL_DIR} PARENT_SCOPE)
set(DEPS_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")
set(CUSTOM_CMAKE_DIR "${CMAKE_SOURCE_DIR}/CMake")

include("${CUSTOM_CMAKE_DIR}/platform.cmake")

# ---- Helpers ---- #
if (ON)

    set(SUBMODULE_UPDATE_CMD ${GIT_CMD} submodule update --depth 1 -f --checkout --init)

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

    setup_dirs("apriltag")
    ExternalProject_Add(
        apriltag
        # Directories
        ${setup_dirs_result}
        BINARY_DIR "${DEPS_BINARY_DIR}/apriltag"
        # Disable download
        DOWNLOAD_COMMAND ${SUBMODULE_UPDATE_CMD} <SOURCE_DIR>
        UPDATE_COMMAND ""
        # CMake handles the rest with default build and install
        CMAKE_ARGS -DBUILD_SHARED_LIBS:BOOL=OFF
            -DBUILD_EXAMPLES:BOOL=OFF -DBUILD_PYTHON_WRAPPER:BOOL=OFF
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -Wno-dev
        # Other options
        EXCLUDE_FROM_ALL ON
    )

endif()

# ---- wxWidgets ---- #
if (ON)

    set(WX_ROOT_DIR "${DEPS_INSTALL_DIR}")
    configure_package_config_file(
        "${CUSTOM_CMAKE_DIR}/wxWidgetsConfig.cmake.in"
        "${DEPS_BINARY_DIR}/wxWidgetsConfig.cmake.out"
        PATH_VARS WX_ROOT_DIR
        INSTALL_DESTINATION "${DEPS_INSTALL_DIR}"
        INSTALL_PREFIX "${DEPS_INSTALL_DIR}")

    set(WX_URL_BASE "https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.5/")
    if (${WIN32})
        set(WX_SOURCE_ZIP "${WX_URL_BASE}/wxWidgets-3.1.5.7z")
    else()
        set(WX_SOURCE_ZIP "${WX_URL_BASE}/wxWidgets-3.1.5.tar.bz2")
    endif()

    set(WX_BUILD_SMALL 
        wxUSE_HTML:BOOL=OFF
        wxUSE_WXHTML_HELP:BOOL=OFF
        wxUSE_XRC:BOOL=OFF
        wxUSE_MEDIACTRL:BOOL=OFF
        wxUSE_RICHTEXT:BOOL=OFF

        wxUSE_URL:BOOL=OFF
        wxUSE_WEBREQUEST:BOOL=OFF
        wxUSE_CONFIG:BOOL=OFF
        wxUSE_SOCKETS:BOOL=OFF
        wxUSE_IPC:BOOL=OFF)

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
        CMAKE_ARGS -DwxBUILD_SHARED:BOOL=OFF 
            -DwxBUILD_MONOLITHIC:BOOL=ON
            -DwxBUILD_OPTIMISE:BOOL=ON
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>

            -DwxBUILD_MSVC_MULTIPROC:BOOL=${MSVC}
            -Wno-dev
            ${WX_BUILD_SMALL}
        # Other options
        STEP_TARGETS install
        EXCLUDE_FROM_ALL ON)

    ExternalProject_Add_Step(wxWidgets install_package
        COMMAND ${CMAKE_COMMAND} -E copy 
            "${DEPS_BINARY_DIR}/wxWidgetsConfig.cmake.out"
            "${DEPS_INSTALL_DIR}/wxWidgetsConfig.cmake"
        DEPENDEES install
    )

endif()

# ---- OpenVR ---- #
if (ON)
    set(OVR_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/openvr")

    set(OVR_INCLUDE_DIR "${DEPS_INSTALL_DIR}/include")
    set(OVR_LIB_OUTPUT "${DEPS_INSTALL_DIR}/bin/openvr_api${PLATFORM_SHARED_EXT}")
    set(OVR_IMPLIB_OUTPUT "${DEPS_INSTALL_DIR}/lib/openvr_api${PLATFORM_INTERFACE_EXT}")

    configure_package_config_file(
        "${CUSTOM_CMAKE_DIR}/openvrConfig.cmake.in" 
        "${DEPS_BINARY_DIR}/openvrConfig.cmake.out"
        PATH_VARS OVR_LIB_OUTPUT OVR_IMPLIB_OUTPUT OVR_INCLUDE_DIR
        INSTALL_DESTINATION "${DEPS_INSTALL_DIR}"
        INSTALL_PREFIX "${DEPS_INSTALL_DIR}")

    set(OVR_LIB_PATH "${PLATFORM_OS1}${PLATFORM_ARCH}/${PLATFORM_LIB_PREFIX}openvr_api")

    add_custom_command(OUTPUT "${DEPS_INSTALL_DIR}/openvrConfig.cmake"
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
        COMMAND ${CMAKE_COMMAND} -E copy
            "${DEPS_BINARY_DIR}/openvrConfig.cmake.out"
            "${DEPS_INSTALL_DIR}/openvrConfig.cmake"
        VERBATIM)
    add_custom_target(openvr DEPENDS "${DEPS_INSTALL_DIR}/openvrConfig.cmake")
endif()

# ---- OpenCV Contrib ---- #
if (ON)

    set(CV_C_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/opencv_contrib")
    set(CV_C_BINARY_DIR "${DEPS_BINARY_DIR}/opencv_contrib")
    add_custom_command(OUTPUT "${CV_C_BINARY_DIR}/modules/aruco/CMakeLists.txt"
        COMMAND ${SUBMODULE_UPDATE_CMD} ${CV_C_PREFIX}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CV_C_PREFIX}/modules/aruco" "${CV_C_BINARY_DIR}/modules/aruco"
        VERBATIM)
    add_custom_target(opencv_contrib DEPENDS "${CV_C_BINARY_DIR}/modules/aruco/CMakeLists.txt")

endif()

# ---- OpenCV ---- #
if (ON)

    set(CV_BUILD_SMALL 
        -DBUILD_opencv_apps:BOOL=OFF
        -DBUILD_JAVA:BOOL=OFF
        -DBUILD_opencv_python3:BOOL=OFF
        -DBUILD_opencv_python_bindings_generator:BOOL=OFF)

    set(CV_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/opencv")
    setup_dirs("opencv")
    ExternalProject_Add(
        opencv
        # Directories
        ${setup_dirs_result}
        BINARY_DIR "${DEPS_BINARY_DIR}/opencv"
        # Disable download
        DOWNLOAD_COMMAND ${SUBMODULE_UPDATE_CMD} <SOURCE_DIR>
        UPDATE_COMMAND ""
        # CMake handles the rest
        CMAKE_ARGS -DBUILD_PERF_TESTS:BOOL=OFF -DBUILD_TESTS:BOOL=OFF
            -DVIDEOIO_PLUGIN_LIST:STRING=ffmpeg
            -DOPENCV_EXTRA_MODULES_PATH:PATH=${CV_C_BINARY_DIR}/modules
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=OFF
            -DBUILD_WITH_STATIC_CRT:BOOL=OFF
            -Wno-dev
            ${CV_BUILD_SMALL}
            
        # Other options
        EXCLUDE_FROM_ALL ON
        DEPENDS opencv_contrib)

endif()
