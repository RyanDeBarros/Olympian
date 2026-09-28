cmake_minimum_required(VERSION 3.20)
project(OlympianTester)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


# ====================
#      Variables
# ====================

# TODO v11 this won't be the case for ENGINE_DIR when building project separately.
set(ENGINE_DIR "${CMAKE_SOURCE_DIR}/engine")
set(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")


# ====================
#    Build Project
# ====================

add_executable(OlympianTester)

target_include_directories(OlympianTester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link engine
target_link_libraries(OlympianTester PUBLIC OlympianEngine)

add_subdirectory(src)

add_custom_command(TARGET OlympianTester POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory
            "$<TARGET_FILE_DIR:OlympianTester>/res"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_DIR}/res"
            "$<TARGET_FILE_DIR:OlympianTester>/res"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_DIR}/OlympianTester.oly"
            "$<TARGET_FILE_DIR:OlympianTester>/Project.oly"

    COMMAND ${CMAKE_COMMAND} -E remove_directory
            "$<TARGET_FILE_DIR:OlympianTester>/internal/shaders"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ENGINE_DIR}/internal/shaders"
            "$<TARGET_FILE_DIR:OlympianTester>/internal/shaders"
)
