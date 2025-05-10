# CMakeList.txt : CMake project for Olympian
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Olympian")

set(LIBRARIES_DIR "D:/Libraries")

# Include directories
include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/engine"
	"${CMAKE_SOURCE_DIR}/gen"
	"${LIBRARIES_DIR}/glew-2.1.0/include"
	"${LIBRARIES_DIR}/glfw-3.4/include"
	"${LIBRARIES_DIR}/glm/include"
	"${LIBRARIES_DIR}/stb/include"
	"${LIBRARIES_DIR}/toml/include"
	"${LIBRARIES_DIR}/nanosvg/include"
)

# Library directories
link_directories(
	"${LIBRARIES_DIR}/glew-2.1.0/lib/Release/x64"
	"${LIBRARIES_DIR}/glfw-3.4/lib-vc2022/x64"
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add source to this project's executable.
file(GLOB_RECURSE ENGINE_FILES "engine/*.cpp" "engine/*.h")
file(GLOB_RECURSE GEN_FILES "gen/*.cpp" "gen/*.h")
file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.h")

add_executable (Olympian ${ENGINE_FILES} ${GEN_FILES} ${SRC_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Olympian PROPERTY CXX_STANDARD 20)
endif()

if (MSVC)
  target_compile_options(Olympian PRIVATE /Zc:__cplusplus)
endif()

# Dependency configuration macros
target_compile_definitions(Olympian PRIVATE GLEW_STATIC)

# Olympian configuration macros
target_compile_definitions(Olympian PRIVATE OLYMPIAN_SUBSHADER_VALIDATION)
target_compile_definitions(Olympian PRIVATE OLYMPIAN_SHADER_VALIDATION)

# Link libraries
target_link_libraries(Olympian
	OpenGL::GL
	glew32s		# link against static GLEW library
	glfw3		# link against GLFW library
)
