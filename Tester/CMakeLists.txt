cmake_minimum_required (VERSION 3.20)
project (OlympianTester)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


# ====================
#      Variables
# ====================

# TODO v9 this won't be the case for ENGINE_DIR when building project separately.
set(ENGINE_DIR ${CMAKE_SOURCE_DIR})
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})


# ====================
#    Build Project
# ====================

add_executable (OlympianTester)

# Project context macros
# TODO v8 Option in editor to re-generate CMakeLists.txt/other auto-generate files if project was moved, or to rename the project.
target_compile_definitions(OlympianTester PUBLIC
	OLYMPIAN_CONTEXT_PROJECT_FILE="${PROJECT_DIR}/Tester.oly"
	OLYMPIAN_CONTEXT_PROJECT_RESOURCE_DIR="${PROJECT_DIR}/res/"
)

target_include_directories(OlympianTester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link engine
target_link_libraries(OlympianTester PUBLIC OlympianEngine)

add_subdirectory(src)
