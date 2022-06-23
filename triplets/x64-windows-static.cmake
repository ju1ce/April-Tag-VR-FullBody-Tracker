set(ENV{VCPKG_DISABLE_METRICS} ON)

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_PLATFORM_TOOLSET "v143")

if (PORT STREQUAL "openvr")
    set(VCPKG_CRT_LINKAGE dynamic)
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()

if (PORT STREQUAL "opencv4")
    set(DBUILD_LIST "aruco,videoio,objdetect,highgui,calib3d" CACHE STRING "" FORCE)
    set(VIDEOIO_PLUGIN_LIST "ffmpeg,gstreamer" CACHE STRING "" FORCE)
endif()

if (PORT STREQUAL "wxwidgets")
    set(WXWIDGETS_USE_STL TRUE)
    set(WXWIDGETS_USE_STD_CONTAINERS TRUE)
endif()
