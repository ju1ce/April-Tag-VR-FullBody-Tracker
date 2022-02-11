set(_PLATFORM_MSVC_VERSIONS    800  900  1000 1020 1100 1200 1300 1310 1400 1500 1600 1700 1800 1900 1910 1911  1912  1913  1914  1915  1916  1920  1921  1922  1923  1924  1925  1926  1927  1928  1929  1930 1931)
set(_PLATFORM_MSVC_TS_VERSIONS 1.2  2.0  4.0  4.2  5.0  6.0  7.0  7.1  8.0  9.0  10.0 11.0 12.0 14.0 14.1 14.11 14.12 14.13 14.14 14.15 14.16 14.20 14.21 14.22 14.23 14.24 14.25 14.26 14.27 14.28 14.29 14.30 14.31)
# set(_PLATFORM_VS_YEAR          0    0    0    0    0    0    2002 2003 2005 2008 2010 2012 2013 2015 2017 2017 2017 2017 2017 2017 2017 2019 2019 2019 2019 2019 2019 2019 2019 2019 2019 2022 2022)

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
if(${WIN32})
    set(PLATFORM_OS "Windows")
    set(PLATFORM_OS1 "win")

    set(PLATFORM_SHARED_EXT ".dll")
    set(PLATFORM_INTERFACE_EXT ".lib")
    set(PLATFORM_STATIC_EXT ".lib")

    set(PLATFORM_LIB_PREFIX "")
elseif(${UNIX} OR ${APPLE})
    if (${APPLE})
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

# Compiler Specific
if (${MSVC})
    LIST(FIND _PLATFORM_MSVC_VERSIONS ${MSVC_VERSION} _PLATFORM_version_index)
    if (${_PLATFORM_version_index} EQUAL -1)
        message(WARNING "Unexpected MSVC_VERSION, maybe it needs to be added to the mapping.")
    else()
        LIST(GET ${_PLATFORM_MSVC_TS_VERSIONS} ${_PLATFORM_version_index} _PLATFORM_version_value)
        set(PLATFORM_MSVC_TOOLSET ${_PLATFORM_version_value})

        if (${_PLATFORM_version_value} VERSION_LESS "14.0")
            string(REPLACE "." "" _PLATFORM_version_short "${_PLATFORM_version_value}")
            set(PLATFORM_MSVC_ABI_COMPAT ${_PLATFORM_version_short})
            unset(_PLATFORM_version_short)
        else()
            set(PLATFORM_MSVC_ABI_COMPAT "14x")
        endif()
        unset(_PLATFORM_version_value)
    endif()
    unset(_PLATFORM_version_index)
endif()
