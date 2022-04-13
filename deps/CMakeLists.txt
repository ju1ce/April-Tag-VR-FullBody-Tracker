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
    -DwxBUILD_SHARED:BOOL=OFF
    -DwxBUILD_OPTIMISE:BOOL=$<AND:$<BOOL:${ENABLE_LTO}>,$<CONFIG:Release>>
    -DwxBUILD_USE_STATIC_RUNTIME:BOOL=$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>
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
    "-DOPENCV_EXTRA_MODULES_PATH:PATH=${CVC_BIN_DIR}/modules" # The modules folder with aruco created by opencv_contrib
    -DBUILD_WITH_STATIC_CRT:BOOL=$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>
    -DENABLE_LTO:BOOL=$<AND:$<BOOL:${ENABLE_LTO}>,$<CONFIG:Release>>
    -DWITH_ITT=OFF
    -DBUILD_ITT=OFF
    -DINSTALL_CREATE_DISTRIB:BOOL=$<BOOL:${BUILD_SHARED_LIBS}>
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
    -DBUILD_SHARED:BOOL=$<BOOL:${BUILD_SHARED_LIBS}>
    -DUSE_LIBCXX:BOOL=OFF
)

# --- libusb ---
set(USB_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/libusb-compiled")
set(USB_INSTALL "${DEPS_INSTALL_DIR}/libusb")

if (MSVC_TOOLSET_VERSION GREATER_EQUAL 143)
    set(USB_VS_VERSION "2022")
elseif (MSVC_TOOLSET_VERSION GREATER_EQUAL 142)
    set(USB_VS_VERSION "2019")
endif()

add_custom_command(
    OUTPUT "${USB_INSTALL}/include/libusb.h" "${USB_INSTALL}/lib/Release/libusb-1.0.lib" "${USB_INSTALL}/lib/Debug/libusb-1.0.lib"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${USB_INSTALL}/include" "${USB_INSTALL}/lib/Release" "${USB_INSTALL}/lib/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy "${USB_PREFIX}/VS${USB_VS_VERSION}/MS64/Release/lib/libusb-1.0.lib" "${USB_INSTALL}/lib/Release/libusb-1.0.lib"
    COMMAND ${CMAKE_COMMAND} -E copy "${USB_PREFIX}/VS${USB_VS_VERSION}/MS64/Debug/lib/libusb-1.0.lib"  "${USB_INSTALL}/lib/Debug/libusb-1.0.lib"
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
set(PS_INSTALL "${DEPS_INSTALL_DIR}/ps3eye")

add_custom_command(
    OUTPUT "${PS_INSTALL}/include/ps3eye.h" "${PS_INSTALL}/src/ps3eye.cpp" "${PS_INSTALL}/include/PSEyeVideoCapture.h" "${PS_INSTALL}/src/PSEyeVideoCapture.cpp"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PS_INSTALL}/include" "${PS_INSTALL}/src"
    COMMAND ${CMAKE_COMMAND} -E copy "${PSD_PREFIX}/ps3eye.h" "${PSMS_PREFIX}/PSEyeVideoCapture.h" "${PS_INSTALL}/include"
    COMMAND ${CMAKE_COMMAND} -E copy "${PSD_PREFIX}/ps3eye.cpp" "${PSMS_PREFIX}/PSEyeVideoCapture.cpp" "${PS_INSTALL}/src"
    VERBATIM
)
add_custom_target(ps3eye-install
    DEPENDS "${PS_INSTALL}/include/ps3eye.h" "${PS_INSTALL}/src/ps3eye.cpp" "${PS_INSTALL}/include/PSEyeVideoCapture.h" "${PS_INSTALL}/src/PSEyeVideoCapture.cpp"
    SOURCES "${PSD_PREFIX}/ps3eye.h" "${PSD_PREFIX}/ps3eye.cpp" "${PSMS_PREFIX}/PSEyeVideoCapture.h" "${PSMS_PREFIX}/PSEyeVideoCapture.cpp")
add_dependencies(ps3eye-install libusb-install)
