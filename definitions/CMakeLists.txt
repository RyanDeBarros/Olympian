find_package(Python3 REQUIRED COMPONENTS Interpreter)

function(prebuild script)
    set(options)
    set(one_value_args)
    set(multi_value_args ARGS)

    cmake_parse_arguments(PB "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m definitions.${script} ${PB_ARGS}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE result
    )

    if (NOT result EQUAL 0)
        message(FATAL_ERROR "definitions/${script} failed with code ${result}")
    endif()
endfunction()

prebuild("KeysCodeGen" ARGS "--key-size=${ASSET_KEY_SIZE}")
