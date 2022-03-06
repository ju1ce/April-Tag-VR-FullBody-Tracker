cmake_minimum_required(VERSION 3.5)

# Used for merging compilation databases, doesnt matter unless you are using clang tools

function(concat_json_files in_files out_file)
    set(out_content "[\n")
    foreach (file ${in_files})
        if (EXISTS ${file})
            file(READ ${file} in_content)
            string(REGEX REPLACE "^[\\t \\n]*\\[" "" json_content_tmp ${in_content})
            string(REGEX REPLACE "\\][\\t \\n]*$" "" json_content ${json_content_tmp})
            string(APPEND out_content ${json_content})
            if (NOT ${json_content} MATCHES ",[\\t \\n]*$")
                string(APPEND out_content ",")
            endif()
        else()
            message(WARNING "File not found: ${file}")
        endif()
    endforeach()
    string(REGEX REPLACE ",[\\t \\n]*$" "" out_content ${out_content})
    string(APPEND out_content "\n]")
    message(STATUS "Writing to ${out_file}")
    file(WRITE ${out_file} ${out_content})
endfunction()

if (NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
    return()
endif()

if (NOT DEFINED IN_FILES)
    message(FATAL_ERROR "Set -DIN_FILES= to list of json files.")
endif()

if (NOT DEFINED OUT_FILE)
    message(FATAL_ERROR "Set -DOUT_FILE= to list of json files.")
endif()

concat_json_files("${IN_FILES}" ${OUT_FILE})