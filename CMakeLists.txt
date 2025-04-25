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
add_executable (Olympian "src/Tester.cpp"  "src/rendering/core/Shaders.h" "src/rendering/core/Shaders.cpp" "src/util/IO.h" "src/util/IO.cpp" "src/util/Errors.h"  "src/rendering/core/Buffers.h" "src/rendering/core/Buffers.cpp" "src/rendering/core/VertexArrays.h" "src/rendering/core/VertexArrays.cpp"  "src/rendering/core/Core.h"  "src/rendering/core/Core.cpp"  "src/rendering/core/Textures.h" "src/rendering/core/Textures.cpp" "src/util/STB.cpp" "src/rendering/core/Window.h" "src/rendering/core/Window.cpp" "src/Olympian.h"  "src/rendering/immutable/Sprites.h" "src/rendering/immutable/Sprites.cpp" "src/rendering/Resources.h" "src/rendering/Resources.cpp" "src/math/DataStructures.h" "src/math/Transforms.h" "src/math/Transforms.cpp" "src/rendering/immutable/Polygons.h" "src/rendering/immutable/Polygons.cpp" "src/math/Geometry.h" "src/math/Geometry.cpp"  "src/math/Triangulation.cpp" "src/util/General.h" "src/util/FixedVector.h"  "src/rendering/SpecializedBuffers.h" "src/util/FreeSpaceTracker.h" "src/util/IDGenerator.h" "src/rendering/immutable/Ellipses.h" "src/rendering/immutable/Ellipses.cpp" "src/util/Logger.h" "src/util/Logger.cpp" "src/util/Assert.h" "src/util/Assert.cpp" "src/util/Colors.h" "src/rendering/Particles.h" "src/rendering/Particles.cpp"  "src/math/Random.h" "src/math/Random.cpp" "src/math/Solvers.h" "src/math/Solvers.cpp" "src/math/Units.h" "src/rendering/Loader.h" "src/rendering/Loader.cpp"   "src/rendering/mutable/Sprites.h" "src/rendering/mutable/Sprites.cpp"  "src/rendering/TextureRegistry.h" "src/rendering/TextureRegistry.cpp" "src/util/NSVG.cpp"   "src/rendering/mutable/SpriteRegistry.h" "src/rendering/mutable/SpriteRegistry.cpp" "src/util/Approximate.h" "src/rendering/immutable/PolygonRegistry.h" "src/rendering/immutable/PolygonRegistry.cpp" "src/rendering/immutable/EllipseRegistry.h" "src/rendering/immutable/EllipseRegistry.cpp" "src/Context.h" "src/Context.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Olympian PROPERTY CXX_STANDARD 20)
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
