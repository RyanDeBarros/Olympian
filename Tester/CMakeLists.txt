cmake_minimum_required (VERSION 3.20)
project (OlympianTester)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Variables

# TODO v7 this won't be the case for ENGINE_DIR when building project separately.
set(ENGINE_DIR "${CMAKE_SOURCE_DIR}")
set(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROJECT_SRC_DIR "${PROJECT_DIR}/src")
set(PROJECT_RES_DIR "${PROJECT_DIR}/res")
set(PROJECT_GEN_DIR "${PROJECT_DIR}/.gen/code")

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
# TODO v7 Option in editor to re-generate CMakeLists.txt/other auto-generate files if project was moved, or to rename the project.
target_compile_definitions(Olympian PUBLIC
	OLYMPIAN_CONTEXT_PROJECT_FILE="${PROJECT_DIR}/Tester.oly"
	OLYMPIAN_CONTEXT_PROJECT_RESOURCE_DIR="${PROJECT_RES_DIR}/"
)

# Link engine
target_link_libraries(Olympian PRIVATE OlympianEngine)
