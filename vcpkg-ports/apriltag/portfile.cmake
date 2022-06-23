vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO AprilRobotics/apriltag
    REF 76dce4c36132a342c36c4d296a6752c6b22a946b
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_build(
    SOURCE_PATH "${SOURCE_PATH}"
)
