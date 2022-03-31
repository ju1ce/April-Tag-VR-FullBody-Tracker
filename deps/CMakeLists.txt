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

att_add_dep(
    openvr

    EXTRA_CMAKE_ARGS
    -DBUILD_SHARED=${BUILD_SHARED_LIBS}
)
