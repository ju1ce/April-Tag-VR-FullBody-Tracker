project(deps)

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
    -DwxUSE_WXHTML_HELP:BOOL=OFF
    -DwxUSE_RICHTEXT:BOOL=OFF
    -DwxUSE_HELP:BOOL=OFF
    -DwxUSE_HTML:BOOL=OFF
    -DwxUSE_XRC:BOOL=OFF
    -DwxUSE_STC:BOOL=OFF
    -DwxUSE_WEBVIEW:BOOL=OFF
    -DwxUSE_SOCKETS:BOOL=OFF
    -DwxUSE_URL:BOOL=OFF
    -DwxUSE_PROTOCOL:BOOL=OFF
    -DwxUSE_FS_INET:BOOL=OFF
    -DwxBUILD_SHARED:BOOL=OFF
    -DwxBUILD_OPTIMISE:BOOL=$<AND:$<BOOL:${ENABLE_LTO}>,$<CONFIG:Release>>
    -DwxBUILD_USE_STATIC_RUNTIME:BOOL=$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>
)
att_configure_package_config(wxWidgets)

# --- OpenCV Contrib ---
att_add_external_project(
    opencv_contrib ""

    DISABLE_CMAKE

    # Create a modules folder in binary dir that only contains aruco, opencv will use this folder
    BUILD_COMMAND ${CMAKE_COMMAND} -E copy_directory
    "<SOURCE_DIR>/modules/aruco" "<BINARY_DIR>/modules/aruco"
)

ExternalProject_Get_Property(opencv_contrib BINARY_DIR)
set(CVC_MODULES_DIR "${BINARY_DIR}/modules")

# --- OpenCV ---
att_add_dep(
    opencv

    EXTRA_CMAKE_ARGS
    -DVIDEOIO_PLUGIN_LIST:STRING=ffmpeg
    -DWITH_OPENGL:BOOL=ON
    "-DOPENCV_EXTRA_MODULES_PATH:PATH=${CVC_MODULES_DIR}" # The modules folder with aruco created by opencv_contrib
    -DBUILD_WITH_STATIC_CRT:BOOL=$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>
    -DENABLE_LTO:BOOL=$<AND:$<BOOL:${ENABLE_LTO}>,$<CONFIG:Release>>
    -DWITH_ITT:BOOL=OFF
    -DBUILD_ITT:BOOL=OFF
    -DINSTALL_CREATE_DISTRIB:BOOL=$<BOOL:${BUILD_SHARED_LIBS}>
    -DOPENCV_GENERATE_SETUPVARS:BOOL=OFF # random file opencv creates that we dont use
    -DBUILD_PERF_TESTS:BOOL=OFF
    -DBUILD_TESTS:BOOL=OFF
    -DBUILD_JAVA:BOOL=OFF
    -DBUILD_DOCS:BOOL=OFF
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_opencv_photo:BOOL=OFF
    -DBUILD_opencv_gapi:BOOL=OFF
    -DBUILD_opencv_apps:BOOL=OFF
    -DBUILD_opencv_python3:BOOL=OFF
    -DBUILD_opencv_python_bindings_generator:BOOL=OFF
    -DBUILD_opencv_js:BOOL=OFF

    DEPENDS opencv_contrib-install
)

# --- OpenVR ---
att_add_dep(
    openvr

    EXTRA_CMAKE_ARGS
    -DBUILD_SHARED:BOOL=$<BOOL:${BUILD_SHARED_LIBS}>
)

# --- libusb ---
set(USB_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/libusb-compiled")
set(USB_INSTALL "${DEPS_INSTALL_DIR}/libusb")

if(MSVC_TOOLSET_VERSION GREATER_EQUAL 143)
    set(USB_VS_VERSION "2022")
elseif(MSVC_TOOLSET_VERSION GREATER_EQUAL 142)
    set(USB_VS_VERSION "2019")
endif()

add_custom_command(
    OUTPUT "${USB_INSTALL}/include/libusb.h" "${USB_INSTALL}/lib/Release/libusb-1.0.lib" "${USB_INSTALL}/lib/Debug/libusb-1.0.lib"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${USB_INSTALL}/include" "${USB_INSTALL}/lib/Release" "${USB_INSTALL}/lib/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy "${USB_PREFIX}/VS${USB_VS_VERSION}/MS64/Release/lib/libusb-1.0.lib" "${USB_INSTALL}/lib/Release/libusb-1.0.lib"
    COMMAND ${CMAKE_COMMAND} -E copy "${USB_PREFIX}/VS${USB_VS_VERSION}/MS64/Debug/lib/libusb-1.0.lib" "${USB_INSTALL}/lib/Debug/libusb-1.0.lib"
    COMMAND ${CMAKE_COMMAND} -E copy "${USB_PREFIX}/include/libusb-1.0/libusb.h" "${USB_INSTALL}/include/libusb.h"
    VERBATIM
)
add_custom_target(libusb-install
    DEPENDS "${USB_INSTALL}/include/libusb.h" "${USB_INSTALL}/lib/Release/libusb-1.0.lib" "${USB_INSTALL}/lib/Debug/libusb-1.0.lib"
    SOURCES "${USB_PREFIX}/include/libusb-1.0/libusb.h"
)

# --- PS3EyeDriver + PS3EyeVideoCapture ---
set(PSD_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/PS3EyeDriver")
set(PSMS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/PSMoveService")

att_add_files_installer(ps3eye-install "${DEPS_INSTALL_DIR}/ps3eye"
    INSTALL_ENTRY "."
    "${PSD_PREFIX}/ps3eyeConfig.cmake"
    INSTALL_ENTRY "include"
    "${PSD_PREFIX}/ps3eye.h"
    "${PSMS_PREFIX}/PSEyeVideoCapture.h"
    INSTALL_ENTRY "src"
    "${PSD_PREFIX}/ps3eye.cpp"
    "${PSMS_PREFIX}/PSEyeVideoCapture.cpp")

add_dependencies(ps3eye-install libusb-install)
