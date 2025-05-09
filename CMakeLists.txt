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
add_executable (Olympian "src/Tester.cpp"  "src/graphics/backend/basic/Shaders.h" "src/graphics/backend/basic/Shaders.cpp" "src/core/util/IO.h" "src/core/util/IO.cpp" "src/core/base/Errors.h"  "src/graphics/backend/basic/Buffers.h" "src/graphics/backend/basic/Buffers.cpp" "src/graphics/backend/basic/VertexArrays.h" "src/graphics/backend/basic/VertexArrays.cpp"      "src/registries/graphics/TextureRegistry.h" "src/graphics/backend/basic/Textures.cpp" "src/external/STB.cpp" "src/core/platform/Window.h" "src/core/platform/Window.cpp" "src/Olympian.h"  "src/graphics/primitives/Sprites.h" "src/graphics/primitives/Sprites.cpp" "src/graphics/resources/Shaders.h" "src/graphics/resources/Shaders.cpp"  "src/core/base/Transforms.h" "src/core/base/Transforms.cpp" "src/graphics/primitives/Polygons.h" "src/graphics/primitives/Polygons.cpp" "src/core/math/Geometry.h" "src/core/math/Geometry.cpp"  "src/core/math/Triangulation.cpp"  "src/core/containers/FixedVector.h"   "src/core/containers/FreeSpaceTracker.h" "src/core/containers/IDGenerator.h" "src/graphics/primitives/Ellipses.h" "src/graphics/primitives/Ellipses.cpp" "src/core/util/Logger.h" "src/core/util/Logger.cpp" "src/core/base/Assert.h" "src/core/base/Assert.cpp" "src/core/base/Colors.h" "src/core/math/Random.h" "src/core/math/Random.cpp" "src/core/math/Solvers.h" "src/core/math/Solvers.cpp" "src/physics/Units.h" "src/registries/Loader.h" "src/registries/Loader.cpp"   "src/external/NSVG.cpp"   "src/registries/graphics/primitives/Sprites.h" "src/registries/graphics/primitives/Sprites.cpp" "src/core/types/Approximate.h" "src/registries/graphics/primitives/Polygons.h" "src/registries/graphics/primitives/Polygons.cpp" "src/registries/graphics/primitives/Ellipses.h" "src/registries/graphics/primitives/Ellipses.cpp" "src/core/base/Context.h" "src/core/base/Context.cpp" "src/graphics/backend/basic/FenceSync.h" "src/core/containers/Ranges.h" "src/graphics/backend/specialized/UsageSlotTracker.h"   "src/graphics/extensions/TileSet.h" "src/graphics/extensions/TileSet.cpp" "src/graphics/extensions/TileMap.h" "src/graphics/extensions/TileMap.cpp" "src/core/util/UTF.h" "src/core/util/UTF.cpp" "src/graphics/text/Font.h" "src/graphics/text/Font.cpp"   "src/graphics/text/Text.h" "src/graphics/text/Text.cpp" "src/graphics/text/Paragraph.h" "src/graphics/text/Paragraph.cpp" "src/core/platform/Events.h" "src/core/platform/Input.h"  "src/core/containers/TreeNode.h" "src/core/platform/Input.cpp"   "src/core/platform/Platform.h" "src/core/platform/Platform.cpp" "src/registries/platform/Input.h" "src/registries/platform/Input.cpp" "src/registries/graphics/text/Paragraphs.h" "src/registries/graphics/text/Paragraphs.cpp"  "src/core/base/SimpleMath.h" "src/core/types/Meta.h"  "src/external/GL.h" "src/external/GLM.h" "src/external/STB.h" "src/external/TOML.h" "src/external/NSVG.h" "src/core/types/STLUtils.h" "src/core/util/Time.h" "src/core/util/Memory.h" "src/graphics/resources/Samplers.h" "src/graphics/resources/Samplers.cpp" "src/graphics/resources/Textures.h" "src/graphics/resources/Textures.cpp" "src/graphics/resources/Resources.h" "src/graphics/resources/Resources.cpp" "src/graphics/Stencil.h" "src/graphics/Stencil.cpp" "src/graphics/backend/basic/Sampler.h" "src/graphics/backend/basic/Sampler.cpp"   "src/registries/graphics/extensions/TileMaps.h" "src/registries/graphics/extensions/TileMaps.cpp" "src/registries/graphics/extensions/TileSetRegistry.h" "src/registries/graphics/extensions/TileSetRegistry.cpp" "src/registries/graphics/text/FontFaceRegistry.h" "src/registries/graphics/text/FontAtlasRegistry.h" "src/registries/graphics/text/FontFaceRegistry.cpp" "src/registries/graphics/text/FontAtlasRegistry.cpp" "src/graphics/backend/specialized/VertexAttributes.h" "src/graphics/backend/specialized/VertexBuffers.h" "src/graphics/backend/specialized/ElementBuffers.h" "src/graphics/backend/specialized/PersistentBuffers.h" "src/graphics/backend/specialized/LazyBuffers.h" "src/graphics/backend/specialized/LightweightBuffers.h" "src/graphics/backend/specialized/Mutability.h" "src/core/math/Triangulation.h" "src/core/math/Coordinates.h" "src/core/math/ColoredGeometry.h" "src/core/math/ColoredGeometry.cpp" "src/core/math/Shapes.h" "src/core/math/Coordinates.cpp" "src/core/math/Shapes.cpp" "src/graphics/extensions/SpriteAtlas.h" "src/graphics/extensions/SpriteAtlas.cpp" "src/core/platform/WindowResize.h" "src/core/platform/WindowResize.cpp" "src/registries/graphics/TextureRegistry.h" "src/registries/graphics/TextureRegistry.cpp" "src/registries/graphics/extensions/SpriteAtlases.h" "src/registries/graphics/extensions/SpriteAtlases.cpp")

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
