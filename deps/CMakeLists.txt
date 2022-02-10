include(ExternalProject)
include(CMakePackageConfigHelpers)

find_program(GIT_CMD git REQUIRED)
find_program(CURL_CMD curl REQUIRED)

set(setup_dirs_result)
macro(setup_dirs prefix)
    set(setup_dirs_result
        PREFIX "${prefix}" SOURCE_DIR "${prefix}" DOWNLOAD_DIR "${prefix}"
        TMP_DIR "${prefix}/build/cmake-tmp" STAMP_DIR "${prefix}/build/cmake-stamp"
        BINARY_DIR "${prefix}/build"
        INSTALL_DIR "${prefix}/install")
endmacro()

set(build_ep_error_result)
macro(build_ep_error msg)
    set(build_ep_error_result
        COMMAND ${CMAKE_COMMAND} -E echo "Dependency Error ${msg}"
        COMMAND ${CMAKE_COMMAND} -E false)
endmacro()

set(AT_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/apriltag")
setup_dirs(${AT_PREFIX})
ExternalProject_Add(
    apriltag
    # Directories
    ${setup_dirs_result}
    # Init the submodule
    DOWNLOAD_COMMAND ${GIT_CMD} submodule update --init <SOURCE_DIR>
    UPDATE_COMMAND ""
    # CMake handles the rest with default build and install
    CMAKE_ARGS -DBUILD_SHARED_LIBS=0 -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    # Other options
    EXCLUDE_FROM_ALL ON
)

set(WX_URL_BASE "https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.5/")
set(WX_HEADER_ZIP "${WX_URL_BASE}wxWidgets-3.1.5-headers.7z")

set(WX_BUILD_FROM_SOURCE OFF CACHE BOOL 
    "Build wxWidgets from source even if a precompiled binary can be downloaded.")
set(WX_BFS ${WX_BUILD_FROM_SOURCE})

# Check if 32 or 64 bit system.
set(SIZEOF_VOIDP ${CMAKE_SIZEOF_VOID_P})
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(WX_ARCH "_x64")
else()
  set(WX_ARCH "")
endif()

# Setup urls for wxWidgets release assets
if (MSVC)
    if (${MSVC_VERSION} LESS 1500)
        set(WX_BFS ON)
    elseif(${MSVC_VERSION} LESS 1600)
        set(WX_MSVC_TS 90)
    elseif(${MSVC_VERSION} LESS 1700)
        set(WX_MSVC_TS 100)
    elseif(${MSVC_VERSION} LESS 1800)
        set(WX_MSVC_TS 110)
    elseif(${MSVC_VERSION} LESS 1900)
        set(WX_MSVC_TS 120)
    else()
        set(WX_MSVC_TS 14x)
    endif()
    
    set(WX_BINARY_ZIP "${WX_URL_BASE}wxMSW-3.1.5_vc${WX_MSVC_TS}${WX_ARCH}_Dev.7z")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_EQUAL 7.3)
        set(WX_GCC_VER 730)
    elseif(${CMAKE_CXX_COMPILER_VERSION} VERSION_EQUAL 8.1)
        set(WX_GCC_VER 810)
    elseif(${CMAKE_CXX_COMPILER_VERSION} VERSION_EQUAL 9.2)
        set(WX_GCC_VER 920TDM)
    else() # if(${CMAKE_CXX_COMPILER_VERSION} VERSION_EQUAL 10.2)
        set(WX_GCC_VER 1020)
    endif()
    
    set(WX_BINARY_ZIP "${WX_URL_BASE}wxMSW-3.1.5_gcc${WX_GCC_VER}${WX_ARCH}_Dev.7z")
else()
    set(WX_BFS ON)
endif()


if (${WX_BFS})
    set(WX_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets")
    setup_dirs(${WX_PREFIX})
    ExternalProject_Add(
        wxWidgets
        # Directories
        ${setup_dirs_result}
        # Init the submodule
        DOWNLOAD_COMMAND ${GIT_CMD} submodule update --recursive --init <SOURCE_DIR>
        UPDATE_COMMAND ""
        # CMake handles the rest with default build and install
        CMAKE_ARGS -DwxBUILD_SHARED=1 -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        # Other options
        EXCLUDE_FROM_ALL ON
    )
else() 
    set(WX_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/precompiled")
    setup_dirs(${WX_PREFIX})

    if (MSVC)
        # Add the wxwidgets props to MSVC projects because it said so
        # set(WX_PROPS "${WX_PREFIX}/wxwidgets.props")
        # configure_file("${CMAKE_SOURCE_DIR}/CMake/wxWidgets.vcxproj.user.in"
        #             "${CMAKE_CURRENT_BINARY_DIR}/wxWidgets.vcxproj.user.IGNORE"
        #             @ONLY)

        # Later when the install step is run for this project, it can rename that file
        # set(WX_MSVC_RENAME_PROPS COMMAND ${CMAKE_COMMAND} -E rename 
        #     "${CMAKE_CURRENT_BINARY_DIR}/wxWidgets.vcxproj.user.IGNORE" 
        #     "${CMAKE_CURRENT_BINARY_DIR}/wxWidgets.vcxproj.user")
    endif()
    # Download precompiled dev package instead of using submodule
    ExternalProject_Add(
        wxWidgets
        # Directories
        ${setup_dirs_result}
        # Download and unpack prebuilt binaries
        DOWNLOAD_COMMAND ${CURL_CMD} -Ls -o "wx_header.zip" "${WX_HEADER_ZIP}"
        COMMAND ${CURL_CMD} -Ls -o "wx_binary.zip" "${WX_BINARY_ZIP}"
        COMMAND ${CMAKE_COMMAND} -E tar x "wx_header.zip"
        COMMAND ${CMAKE_COMMAND} -E tar x "wx_binary.zip"
        ${WX_MSVC_RENAME_PROPS}
        # Disable other steps
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        # Other options
        EXCLUDE_FROM_ALL ON
        TLS_VERIFY ON
    )
endif()

set(OVR_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/openvr")

configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/CMake/openvrConfig.cmake.in" 
    "${OVR_PREFIX}/openvrConfig.cmake"
    INSTALL_DESTINATION "${OVR_PREFIX}/openvrConfig.cmake")

setup_dirs(${OVR_PREFIX})
ExternalProject_Add(
    openvr
    # Directories
    ${setup_dirs_result}
    # Get submodule files 
    DOWNLOAD_COMMAND ${GIT_CMD} submodule update --init <SOURCE_DIR>
    # Create a fake install
    ${OVR_INSTALL_CMDS}
    # Disable other steps
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    # Other options
    EXCLUDE_FROM_ALL ON
)