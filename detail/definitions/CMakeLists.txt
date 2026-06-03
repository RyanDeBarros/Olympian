find_package(Python3 REQUIRED COMPONENTS Interpreter)

execute_process(
    COMMAND ${Python3_EXECUTABLE} KeysCodeGen.py
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE result
)

if (NOT result EQUAL 0)
    message(FATAL_ERROR "KeysCodeGen.py failed with code ${result}")
endif()

add_subdirectory(enums)
