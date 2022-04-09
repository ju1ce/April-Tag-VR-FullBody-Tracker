
add_compile_options(
    -fuse-ld=lld-link

    -fms-compatibility
    -fms-extensions
    -msse
    -msse2
    -msse3
    -msse4.1
    -msse4.2
    -mssse3
    -mavx2)

if(PROJECT_NAME STREQUAL "AprilTagTrackers")
    add_compile_options(
        /W4
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        /permissive-
        $<$<CONFIG:Release>:-flto>
        $<$<CONFIG:Release>:-fvirtual-function-elimination>
        $<$<CONFIG:Release>:-fwhole-program-vtables>
        $<$<CONFIG:Release>:-clang:-ffast-math>)
endif()

if(PROJECT_NAME STREQUAL "OpenCV")
    add_compile_options(
        -Wno-unused-variable
        -Wno-unused-but-set-variable
        -Wno-null-pointer-subtraction
        -Wno-undef
        -Wno-macro-redefined
        -Wno-unknown-argument
        -Wno-unused-command-line-argument
        -Wno-unused-parameter
        -Wno-missing-prototypes
        -Wno-sign-compare
        -Wno-inconsistent-missing-override
        -Wno-missing-field-initializers)
endif()

if(PROJECT_NAME STREQUAL "openvr")
    add_compile_options(
        -Wno-deprecated-declarations
    )
endif()

if(PROJECT_NAME STREQUAL "wxWidgets")
    add_compile_options(
        -Wno-deprecated-declarations
        -Wno-unused-local-typedef
        -Wno-unused-but-set-variable
    )
endif()
