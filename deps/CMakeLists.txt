set_directory_properties(PROPERTIES EP_UPDATE_DISCONNECTED TRUE)

# --- AprilTag ---
att_add_dep(
    apriltag

    EXTRA_CMAKE_ARGS
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_PYTHON_WRAPPER:BOOL=OFF
)

# --- wxWidgets ---
# TODO: Allow picking preinstalled on linux as its usually available
# Should be compatible with version 3.1 and up
att_add_dep(
    wxWidgets

    EXTRA_CMAKE_ARGS
    -DwxBUILD_OPTIMISE:BOOL=$<CONFIG:Release>
)
att_configure_package_config(wxWidgets)

# --- OpenCV Contrib ---
set(CVC_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/opencv_contrib")
set(CVC_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/opencv_contrib")

# --- OpenCV ---
att_add_dep(
    opencv

    EXTRA_CMAKE_ARGS
    -DVIDEOIO_PLUGIN_LIST:STRING=ffmpeg
    -DOPENCV_EXTRA_MODULES_PATH:PATH=${CVC_BIN_DIR}/modules # The modules folder with aruco created by opencv_contrib
    -DBUILD_WITH_STATIC_CRT:BOOL=OFF # multiple static libs using different versions of static crt leads to issues
    -DENABLE_LTO:BOOL=$<CONFIG:Release>
    -DWITH_ITT=OFF
    -DBUILD_ITT=OFF
    -DINSTALL_CREATE_DISTRIB:BOOL=${BUILD_SHARED_LIBS}
    -DOPENCV_GENERATE_SETUPVARS:BOOL=OFF # random file opencv creates that we dont use
    -DBUILD_PERF_TESTS:BOOL=OFF
    -DBUILD_TESTS:BOOL=OFF
    -DBUILD_JAVA:BOOL=OFF
    -DBUILD_DOCS:BOOL=OFF
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_opencv_apps:BOOL=OFF
    -DBUILD_opencv_python3:BOOL=OFF
    -DBUILD_opencv_python_bindings_generator:BOOL=OFF
    -DBUILD_opencv_js:BOOL=OFF
)

ExternalProject_Add_Step(opencv aruco DEPENDERS configure
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CVC_PREFIX}/modules/aruco" "${CVC_BIN_DIR}/modules/aruco")

# --- OpenVR ---

# TODO: switch to funnbot/openvr and build statically
set(OVR_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/openvr")
set(OVR_INSTALL_DIR "${DEPS_INSTALL_DIR}/openvr")

if(WIN32)
    set(OVR_OS "win")
    set(OVR_LINK_EXT ".lib")
    set(OVR_SHARED_EXT ".dll")
    unset(OVR_NAME_PREFIX)
elseif(UNIX)
    set(OVR_OS "linux")
    set(OVR_LINK_EXT ".so")
    set(OVR_SHARED_EXT ".so")
    set(OVR_NAME_PREFIX "lib")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "64")
    set(OVR_ARCH "64")
else()
    set(OVR_ARCH "32")
endif()

set(OVR_SHARED_FILE "${OVR_PREFIX}/bin/${OVR_OS}${OVR_ARCH}/${OVR_NAME_PREFIX}openvr_api${OVR_SHARED_EXT}")
set(OVR_LINK_FILE "${OVR_PREFIX}/lib/${OVR_OS}${OVR_ARCH}/${OVR_NAME_PREFIX}openvr_api${OVR_LINK_EXT}")
set(OVR_INCLUDE_FILES
    "${OVR_PREFIX}/headers/openvr.h"
    "${OVR_PREFIX}/headers/openvr_driver.h")

get_filename_component(OVR_SHARED_NAME "${OVR_SHARED_FILE}" NAME)
get_filename_component(OVR_LINK_NAME "${OVR_LINK_FILE}" NAME)

set(OVR_INSTALL_INCLUDE_DIR "${OVR_INSTALL_DIR}/include")
set(OVR_INSTALL_SHARED_FILE "${OVR_INSTALL_DIR}/bin/${OVR_SHARED_NAME}")
set(OVR_INSTALL_LINK_FILE "${OVR_INSTALL_DIR}/lib/${OVR_LINK_NAME}")

find_program(GIT_CMD git REQUIRED DOC "Checkout submodules.")

# fake install of the files from the repo, into a standard-ish layout
add_custom_command(OUTPUT "${OVR_INSTALL_SHARED_FILE}"
    COMMAND "${GIT_CMD}" submodule update --init --depth 1 --checkout "${OVR_PREFIX}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory ${OVR_INSTALL_INCLUDE_DIR}
    COMMAND "${CMAKE_COMMAND}" -E copy ${OVR_INCLUDE_FILES} "${OVR_INSTALL_INCLUDE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${OVR_SHARED_FILE}" "${OVR_INSTALL_SHARED_FILE}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${OVR_LINK_FILE}" "${OVR_INSTALL_LINK_FILE}"
    VERBATIM)

add_custom_target(openvr-install DEPENDS
    "${OVR_INSTALL_SHARED_FILE}")

att_configure_package_config(openvr PATH_VARS
    OVR_INSTALL_INCLUDE_DIR
    OVR_INSTALL_SHARED_FILE
    OVR_INSTALL_LINK_FILE)
