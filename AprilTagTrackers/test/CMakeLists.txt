cmake_minimum_required(VERSION 3.16)
project(test CXX)

add_executable(att_test)

target_sources(att_test PRIVATE
    main.cpp

    "${ATT_DIR}/AprilTagWrapper.cpp"
    "${ATT_DIR}/Connection.cpp"
    "${ATT_DIR}/Helpers.cpp"
    "${ATT_DIR}/Quaternion.cpp"
    "${ATT_DIR}/Serializable.cpp"
    "${ATT_DIR}/Tracker.cpp"
    "${ATT_DIR}/Debug.cpp"

    "${ATT_DIR}/ps3eye/ps3eye.cpp"
    "${ATT_DIR}/ps3eye/PSEyeVideoCapture.cpp"

    GUI_stub.cpp)

if (WIN32)
    target_sources(att_test PRIVATE
        "${ATT_DIR}/IPC/WindowsNamedPipe.cpp"
    )
else()
    target_sources(att_test PRIVATE
        "${ATT_DIR}/IPC/UNIXSocket.cpp"
    )
endif()

if (MSVC)
    if (BUILD_SHARED_LIBS)
        att_target_crt_linkage(att_test DYNAMIC)
    else()
        att_target_crt_linkage(att_test STATIC)
    endif()
endif()

target_link_libraries(att_test PRIVATE
    ${OpenCV_LIBRARIES}
    ${LIBUSB_LIBRARIES}
    apriltag::apriltag
    openvr::openvr_api
    doctest::doctest
    common::semver
)
target_include_directories(att_test SYSTEM PRIVATE
    ${LIBUSB_INCLUDE_DIRS}
    "${ATT_DIR}/ps3eye"
)

# Set the root of includes, rather than relative
target_include_directories(att_test PRIVATE
    "${ATT_DIR}"
)
# Ensure compiler with c++17 language features
target_compile_features(att_test PRIVATE cxx_std_17)
# Create debug symbols for release builds, msvc will generate a pdb,
# while gcc-like will have embedded symbols.
att_exe_debug_symbols(att_test)

target_compile_definitions(att_test PRIVATE
    ATT_DRIVER_VERSION=${DRIVER_VERSION}
    ATT_LOG_LEVEL=3
    ATT_OVERRIDE_ERROR_HANDLERS
    ATT_ENABLE_ASSERT
    ATT_ENABLE_PS3EYE
)
# Set cross-platform defines in sources
if(WIN32)
    target_compile_definitions(att_test PRIVATE "OS_WIN=1")
elseif(UNIX)
    target_compile_definitions(att_test PRIVATE "OS_LINUX=1")
else()
    message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}, compilation will fail.")
endif()
# Build with Address Sanitizer
if(ATT_TEST_ENABLE_ASAN)
    att_target_enable_asan(att_test)
endif()

# Install application to bin folder
install(TARGETS att_test RUNTIME DESTINATION ".")
if(WIN32)
    # install pdb file for debugging
    install(FILES $<TARGET_PDB_FILE:att_test> DESTINATION "." OPTIONAL)
endif()
