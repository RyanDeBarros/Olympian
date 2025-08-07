cmake_minimum_required (VERSION 3.20)
project (OlympianTester)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Run automatic tools
find_package(Python3 REQUIRED COMPONENTS Interpreter)

execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tools/PreBuild.py"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res"
	RESULT_VARIABLE PREBUILD_RESULT
)

if (NOT PREBUILD_RESULT EQUAL 0)
	message(FATAL_ERROR "PreBuild.py failed with code ${PREBUILD_RESULT}")
endif()

# Variables

set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PROJECT_GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.gen")

# Include directories
include_directories(
	"${PROJECT_SRC_DIR}"
	"${PROJECT_GEN_DIR}"
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add source to this project's executable.
file(GLOB_RECURSE SRC_FILES "${PROJECT_SRC_DIR}/*.cpp" "${PROJECT_SRC_DIR}/*.h")
file(GLOB_RECURSE GEN_FILES "${PROJECT_GEN_DIR}/*.cpp" "${PROJECT_GEN_DIR}/*.h")

# Define executable
add_executable (Olympian ${GEN_FILES} ${SRC_FILES})

# Project context macros
target_compile_definitions(Olympian PUBLIC
	OLYMPIAN_CONTEXT_PROJECT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/Tester.oly"
	OLYMPIAN_CONTEXT_PROJECT_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/res/"
)

# Link engine
target_link_libraries(Olympian PRIVATE OlympianEngine)
