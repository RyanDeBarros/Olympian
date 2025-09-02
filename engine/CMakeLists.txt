cmake_minimum_required(VERSION 3.20)
project(OlympianEngine VERSION 1.0.0 LANGUAGES CXX)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Variables
set(LIBRARIES_DIR "D:/Libraries")

# Add Quartic static library
add_library(Quartic STATIC "${LIBRARIES_DIR}/Quartic/src/quartic.cpp")
target_include_directories(Quartic PUBLIC "${LIBRARIES_DIR}/Quartic/include")

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add engine sources
file(GLOB_RECURSE ENGINE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# TODO v4 licensing may require engine to be dynamically linked?
# Create the engine as a static library
add_library(OlympianEngine STATIC ${ENGINE_FILES})

# Include directories
target_include_directories(OlympianEngine PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${LIBRARIES_DIR}/glew-2.1.0/include"
	"${LIBRARIES_DIR}/glfw-3.4/include"
	"${LIBRARIES_DIR}/glm/include"
	"${LIBRARIES_DIR}/stb/include"
	"${LIBRARIES_DIR}/toml/include"
	"${LIBRARIES_DIR}/nanosvg/include"
)

# Link directories
target_link_directories(OlympianEngine PUBLIC
	"${LIBRARIES_DIR}/glew-2.1.0/lib/Release/x64"
	"${LIBRARIES_DIR}/glfw-3.4/lib-vc2022/x64"
)

# Engine compile definitions
target_compile_definitions(OlympianEngine PUBLIC
	GLEW_STATIC
    OLYMPIAN_ENGINE_ABS_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

# Engine compile options (MSVC specific)
if (MSVC)
	target_compile_options(OlympianEngine PRIVATE /Zc:__cplusplus)
endif()

# Link engine dependencies
target_link_libraries(OlympianEngine PUBLIC
	OpenGL::GL
	glew32s
	glfw3
	Quartic
)

# TODO v4 install OlympianEngine so as to use find_package(Olympian) in project cmake
