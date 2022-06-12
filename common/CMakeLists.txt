# common sources shared by our projects
project(common CXX)

# add_common_header(<target_name> <headers...>)
# creates target common::<target_name>
function(add_common_header target_name)
    add_library(common_${target_name} INTERFACE)
    target_include_directories(common_${target_name} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_sources(common_${target_name} INTERFACE ${ARGN})
    add_library(common::${target_name} ALIAS common_${target_name})
endfunction()

# add_common(<target_name> <sources...>)
# creates target common::<target_name>
function(add_common target_name)
    add_library(common_${target_name} OBJECT ${ARGN})
    target_include_directories(common_${target_name} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
    add_library(common::${target_name} ALIAS common_${target_name})
endfunction()
