# CMakeList.txt : CMake project for Olympian, include source and define
# project specific logic here.
#
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
	"${LIBRARIES_DIR}/glew-2.1.0/include"
	"${LIBRARIES_DIR}/glfw-3.4/include"
	"${LIBRARIES_DIR}/glm/include"
	"${LIBRARIES_DIR}/stb/include"
)

# Library directories
link_directories(
	"${LIBRARIES_DIR}/glew-2.1.0/lib/Release/x64"
	"${LIBRARIES_DIR}/glfw-3.4/lib-vc2022/x64"
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add source to this project's executable.
add_executable (Olympian "src/Tester.cpp"  "src/rendering/core/Shaders.h" "src/rendering/core/Shaders.cpp" "src/util/IO.h" "src/util/IO.cpp" "src/util/Errors.h"  "src/rendering/core/Buffers.h" "src/rendering/core/Buffers.cpp" "src/rendering/core/VertexArrays.h" "src/rendering/core/VertexArrays.cpp"  "src/rendering/core/Batches.h" )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Olympian PROPERTY CXX_STANDARD 20)
endif()

# Define GLEW_STATIC before compiling
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
