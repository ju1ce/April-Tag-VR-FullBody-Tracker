include(ExternalProject)
include(CMakePackageConfigHelpers)
include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/platform.cmake")

# region Helpers
find_program(GIT_CMD git DOC "Git is used to checkout submodules")
set(SUBMODULE_UPDATE_CMD ${GIT_CMD} submodule update --init --checkout)

# common external project args
set(COMMON_EP_ARGS
    ${EP_DO_MULTI_CONFIG}
    EXCLUDE_FROM_ALL ON
    UPDATE_COMMAND ""
    STEP_TARGETS install)

# Macro to set the standard layout of external projects
function(setup_dirs namespace libname)
    cmake_parse_arguments(_a "" "SOURCE_DIR" "" ${ARGN})

    if(DEFINED _a_SOURCE_DIR)
        set(libsource "${_a_SOURCE_DIR}")
    else()
        set(libsource "${CMAKE_CURRENT_SOURCE_DIR}/${libname}")
    endif()

    set(libbuild "${DEPS_BINARY_DIR}/${libname}")
    set(${namespace}_PREFIX "${libsource}" PARENT_SCOPE)
    set(${namespace}_BIN_DIR "${libbuild}" PARENT_SCOPE)

    set(setup_dirs_result
        PREFIX "${libbuild}"
        SOURCE_DIR "${libsource}"
        DOWNLOAD_DIR "${libsource}"
        BINARY_DIR "${libbuild}"
        TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/${libname}-tmp"
        STAMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/${libname}-stamp"
        INSTALL_DIR "${DEPS_INSTALL_DIR}"
        PARENT_SCOPE)
endfunction()

function(configure_install_file libname)
    cmake_parse_arguments(_a "" "" "PATH_VARS" ${ARGN})
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/CMake/${libname}.cmake.in"
        "${DEPS_INSTALL_DIR}/${libname}.cmake"
        PATH_VARS ${_a_PATH_VARS}
        INSTALL_DESTINATION "${DEPS_INSTALL_DIR}"
        INSTALL_PREFIX "${DEPS_INSTALL_DIR}")
endfunction()

# endregion Helpers

# region AprilTag
setup_dirs(AT "apriltag")
ExternalProject_Add(
    apriltag
    ${setup_dirs_result}
    DOWNLOAD_COMMAND ${SUBMODULE_UPDATE_CMD} <SOURCE_DIR>

    # CMake handles the rest with default build and install
    CMAKE_ARGS ${COMMON_CMAKE_ARGS} -DBUILD_EXAMPLES:BOOL=OFF -DBUILD_PYTHON_WRAPPER:BOOL=OFF
    ${COMMON_EP_ARGS}
)

# endregion AprilTag

# region wxWidgets
if(WX_DOWNLOAD_MINIMAL)
    # Download and unpack OS specific archives, into build/wxWidgets/src,
    # then run cmake normally within
    setup_dirs(WX "wxWidgets" SOURCE_DIR "${DEPS_BINARY_DIR}/wxWidgets/src")
    set(WX_VERSION "3.1.5")
    set(WX_URL_BASE "https://github.com/wxWidgets/wxWidgets/releases/download/v${WX_VERSION}/")

    find_program(CURL_CMD curl REQUIRED DOC "Download wxWidgets source.")

    if(WIN32)
        set(WX_SOURCE_ZIP "${WX_URL_BASE}/wxWidgets-${WX_VERSION}.7z")
    else()
        # linux unpacks into a sub directory
        set(WX_SOURCE_ZIP "${WX_URL_BASE}/wxWidgets-${WX_VERSION}.tar.bz2")
        set(WX_MOVE_SOURCE_LINUX
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${WX_PREFIX}/wxWidgets-${WX_VERSION}" "${WX_PREFIX}/"
            COMMAND ${CMAKE_COMMAND} -E remove_directory
            "${WX_PREFIX}/wxWidgets-${WX_VERSION}")
    endif()

    set(WX_DL_CMD
        DOWNLOAD_COMMAND ${CURL_CMD} -Ls -o "wx_source.7z" "${WX_SOURCE_ZIP}"
        COMMAND ${CMAKE_COMMAND} -E tar x "wx_source.7z"
        ${WX_MOVE_SOURCE_LINUX})
else()
    setup_dirs(WX "wxWidgets")
    set(WX_DL_CMD DOWNLOAD_COMMAND ${SUBMODULE_UPDATE_CMD} <SOURCE_DIR>)
endif()

ExternalProject_Add(
    wxWidgets
    ${setup_dirs_result}

    # Download or checkout
    ${WX_DL_CMD}

    # CMake handles the rest with default build and install
    CMAKE_ARGS ${COMMON_CMAKE_ARGS} -DwxBUILD_OPTIMISE:BOOL=$<CONFIG:Release>

    ${COMMON_EP_ARGS}
)

configure_install_file("wxWidgetsConfig" PATH_VARS DEPS_INSTALL_DIR)
set_property(TARGET wxWidgets-install APPEND PROPERTY SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/wxWidgetsConfig.cmake.in")

# endregion wxWidgets

# region OpenVR
# TODO: switch to funnbot/openvr and build statically
setup_dirs(OVR "openvr")

set(OVR_INCLUDE_DIR "${DEPS_INSTALL_DIR}/include")
set(OVR_LIB_OUTPUT "${DEPS_INSTALL_DIR}/bin/openvr_api${PLATFORM_SHARED_EXT}")
set(OVR_IMPLIB_OUTPUT "${DEPS_INSTALL_DIR}/lib/openvr_api${PLATFORM_INTERFACE_EXT}")
configure_install_file("openvrConfig"
    PATH_VARS OVR_INCLUDE_DIR OVR_LIB_OUTPUT OVR_IMPLIB_OUTPUT)

# fake install of the files from the repo, into a standard layout and correct location
set(OVR_LIB_PATH "${PLATFORM_OS1}${PLATFORM_ARCH}/${PLATFORM_LIB_PREFIX}openvr_api")
add_custom_command(OUTPUT "${OVR_LIB_OUTPUT}"
    COMMAND ${SUBMODULE_UPDATE_CMD} ${OVR_PREFIX}
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${OVR_INCLUDE_DIR}

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
add_custom_target(openvr-install DEPENDS
    "${OVR_LIB_OUTPUT}"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/openvrConfig.cmake.in")

# endregion OpenVR

# region OpenCV Contrib
setup_dirs(CV_C "opencv_contrib")

# Copies the aruco module to a folder in build,
# where opencv can reference when building
add_custom_command(OUTPUT "${CV_C_BIN_DIR}/modules/aruco/CMakeLists.txt"
    COMMAND ${SUBMODULE_UPDATE_CMD} ${CV_C_PREFIX}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CV_C_PREFIX}/modules/aruco" "${CV_C_BIN_DIR}/modules/aruco"

    VERBATIM)

# using the cmakelists file as dependency to link to opencv
add_custom_target(opencv_contrib DEPENDS "${CV_C_BIN_DIR}/modules/aruco/CMakeLists.txt")

# endregion OpenCV Contrib

# region OpenCV
# remove some unnecessary build outputs,
# hopefully decreases build time
set(CV_BUILD_SMALL
    -DBUILD_opencv_apps:BOOL=OFF
    -DBUILD_JAVA:BOOL=OFF
    -DBUILD_opencv_python3:BOOL=OFF
    -DBUILD_opencv_python_bindings_generator:BOOL=OFF
    -DBUILD_PERF_TESTS:BOOL=OFF
    -DBUILD_TESTS:BOOL=OFF)

setup_dirs(CV "opencv")
ExternalProject_Add(
    opencv
    ${setup_dirs_result}

    # Fetch the submodule files if needed
    DOWNLOAD_COMMAND ${SUBMODULE_UPDATE_CMD} <SOURCE_DIR>

    # CMake handles the rest
    CMAKE_ARGS
    ${COMMON_CMAKE_ARGS}
    -DVIDEOIO_PLUGIN_LIST:STRING=ffmpeg
    -DOPENCV_EXTRA_MODULES_PATH:PATH=${CV_C_BIN_DIR}/modules # The modules folder with aruco created by opencv_contrib
    -DBUILD_WITH_STATIC_CRT:BOOL=${BUILD_SHARED_LIBS} # multiple static libs using different versions of static crt leads to issues
    -DENABLE_LTO:BOOL=$<CONFIG:Release>
    -DWITH_OPENGL:BOOL=ON
    -DWITH_ITT=OFF
    -DBUILD_ITT=OFF
    -DINSTALL_CREATE_DISTRIB:BOOL=${BUILD_SHARED_LIBS}
    -DOPENCV_GENERATE_SETUPVARS:BOOL=OFF # random file opencv creates that we dont use
    ${CV_BUILD_SMALL}

    DEPENDS opencv_contrib
    ${COMMON_EP_ARGS}
)

# endregion OpenCV
