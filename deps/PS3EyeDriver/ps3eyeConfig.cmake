set(PS3EYE_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(LIBUSB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../libusb")

# local scope library, only included by ps3eye
add_library(ps3eye::libusb STATIC IMPORTED
    "${LIBUSB_ROOT}/include/libusb.h")
set_target_properties(ps3eye::libusb PROPERTIES
    IMPORTED_LOCATION_DEBUG "${LIBUSB_ROOT}/lib/Debug/libusb-1.0.lib"
    IMPORTER_LOCATION_RELEASE "${LIBUSB_ROOT}/lib/Release/libusb-1.0.lib")
target_include_directories(ps3eye::libusb SYSTEM INTERFACE
    "${LIBUSB_ROOT}/include")

# interface library sources added here are private
add_library(ps3eye::ps3eye INTERFACE)
target_include_directories(ps3eye::ps3eye SYSTEM INTERFACE
    "${PS3EYE_ROOT}/include")
# instead make them interface sources
target_sources(ps3eye::ps3eye INTERFACE
    "${PS3EYE_ROOT}/src/ps3eye.cpp"
    "${PS3EYE_ROOT}/include/ps3eye.h"

    "${PS3EYE_ROOT}/src/PSEyeVideoCapture.cpp"
    "${PS3EYE_ROOT}/include/PSEyeVideoCapture.h")
