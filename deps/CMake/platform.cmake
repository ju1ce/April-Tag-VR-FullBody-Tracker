macro(clean_platform)
    unset(PLATFORM_ARCH)
    unset(PLATFORM_ARCH1)
    unset(PLATFORM_ARCH2)

    unset(PLATFORM_OS)
    unset(PLATFORM_OS1)

    unset(PLATFORM_SHARED_EXT)
    unset(PLATFORM_INTERFACE_EXT)
    unset(PLATFORM_STATIC_EXT)

    unset(PLATFORM_LIB_PREFIX)

    unset(PLATFORM_MSVC_TOOLSET)
    unset(PLATFORM_MSVC_ABI_COMPAT)
endmacro(clean_platform)

clean_platform()

# Check if 32 or 64 bit system.
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(PLATFORM_ARCH "64")
    set(PLATFORM_ARCH1 "x64")
    set(PLATFORM_ARCH2 "x86_64")
else()
    set(PLATFORM_ARCH "32")
    set(PLATFORM_ARCH1 "x86")
    set(PLATFORM_ARCH2 "x86")
endif()

# OS Specific
if(WIN32)
    set(PLATFORM_OS "Windows")
    set(PLATFORM_OS1 "win")

    set(PLATFORM_SHARED_EXT ".dll")
    set(PLATFORM_INTERFACE_EXT ".lib")
    set(PLATFORM_STATIC_EXT ".lib")

    set(PLATFORM_LIB_PREFIX "")
elseif(UNIX)
    if (APPLE)
        set(PLATFORM_OS "MacOS")
        set(PLATFORM_OS1 "osx")

        set(PLATFORM_SHARED_EXT ".dylib")
        set(PLATFORM_INTERFACE_EXT ".dylib")
    else()
        set(PLATFORM_OS "Linux")
        set(PLATFORM_OS1 "linux")

        set(PLATFORM_SHARED_EXT ".so")
        set(PLATFORM_INTERFACE_EXT ".so")
    endif()

    set(PLATFORM_STATIC_EXT ".a")
    set(PLATFORM_LIB_PREFIX "lib")
endif()
