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
add_executable (Olympian "src/Tester.cpp"  "engine/graphics/backend/basic/Shaders.h" "engine/graphics/backend/basic/Shaders.cpp" "engine/core/util/IO.h" "engine/core/util/IO.cpp" "engine/core/base/Errors.h"  "engine/graphics/backend/basic/Buffers.h" "engine/graphics/backend/basic/Buffers.cpp" "engine/graphics/backend/basic/VertexArrays.h" "engine/graphics/backend/basic/VertexArrays.cpp"      "engine/registries/graphics/TextureRegistry.h" "engine/graphics/backend/basic/Textures.cpp" "engine/external/STB.cpp" "engine/core/platform/Window.h" "engine/core/platform/Window.cpp" "engine/Olympian.h"  "engine/graphics/primitives/Sprites.h" "engine/graphics/primitives/Sprites.cpp" "engine/graphics/resources/Shaders.h" "engine/graphics/resources/Shaders.cpp"  "engine/core/base/Transforms.h" "engine/core/base/Transforms.cpp" "engine/graphics/primitives/Polygons.h" "engine/graphics/primitives/Polygons.cpp" "engine/core/math/Geometry.h" "engine/core/math/Geometry.cpp"  "engine/core/math/Triangulation.cpp"  "engine/core/containers/FixedVector.h"   "engine/core/containers/FreeSpaceTracker.h" "engine/core/containers/IDGenerator.h" "engine/graphics/primitives/Ellipses.h" "engine/graphics/primitives/Ellipses.cpp" "engine/core/util/Logger.h" "engine/core/util/Logger.cpp" "engine/core/base/Assert.h" "engine/core/base/Assert.cpp" "engine/core/base/Colors.h" "engine/core/math/Random.h" "engine/core/math/Random.cpp" "engine/core/math/Solvers.h" "engine/core/math/Solvers.cpp" "engine/physics/Units.h" "engine/registries/Loader.h" "engine/registries/Loader.cpp"   "engine/external/NSVG.cpp"   "engine/registries/graphics/primitives/Sprites.h" "engine/registries/graphics/primitives/Sprites.cpp" "engine/core/types/Approximate.h" "engine/registries/graphics/primitives/Polygons.h" "engine/registries/graphics/primitives/Polygons.cpp" "engine/registries/graphics/primitives/Ellipses.h" "engine/registries/graphics/primitives/Ellipses.cpp" "engine/core/base/Context.h" "engine/core/base/Context.cpp" "engine/graphics/backend/basic/FenceSync.h" "engine/core/containers/Ranges.h" "engine/graphics/backend/specialized/UsageSlotTracker.h"   "engine/graphics/extensions/TileSet.h" "engine/graphics/extensions/TileSet.cpp" "engine/graphics/extensions/TileMap.h" "engine/graphics/extensions/TileMap.cpp" "engine/core/util/UTF.h" "engine/core/util/UTF.cpp" "engine/graphics/text/Font.h" "engine/graphics/text/Font.cpp"   "engine/graphics/text/Text.h" "engine/graphics/text/Text.cpp" "engine/graphics/text/Paragraph.h" "engine/graphics/text/Paragraph.cpp" "engine/core/platform/Events.h" "engine/core/platform/Input.h"  "engine/core/containers/TreeNode.h" "engine/core/platform/Input.cpp"   "engine/core/platform/Platform.h" "engine/core/platform/Platform.cpp" "engine/registries/platform/Input.h" "engine/registries/platform/Input.cpp" "engine/registries/graphics/text/Paragraphs.h" "engine/registries/graphics/text/Paragraphs.cpp"  "engine/core/base/SimpleMath.h" "engine/core/types/Meta.h"  "engine/external/GL.h" "engine/external/GLM.h" "engine/external/STB.h" "engine/external/TOML.h" "engine/external/NSVG.h" "engine/core/types/STLUtils.h" "engine/core/util/Time.h" "engine/core/util/Memory.h" "engine/graphics/resources/Samplers.h" "engine/graphics/resources/Samplers.cpp" "engine/graphics/resources/Textures.h" "engine/graphics/resources/Textures.cpp" "engine/graphics/resources/Resources.h" "engine/graphics/resources/Resources.cpp" "engine/graphics/Stencil.h" "engine/graphics/Stencil.cpp" "engine/graphics/backend/basic/Sampler.h" "engine/graphics/backend/basic/Sampler.cpp"   "engine/registries/graphics/extensions/TileMaps.h" "engine/registries/graphics/extensions/TileMaps.cpp" "engine/registries/graphics/extensions/TileSetRegistry.h" "engine/registries/graphics/extensions/TileSetRegistry.cpp" "engine/registries/graphics/text/FontFaceRegistry.h" "engine/registries/graphics/text/FontAtlasRegistry.h" "engine/registries/graphics/text/FontFaceRegistry.cpp" "engine/registries/graphics/text/FontAtlasRegistry.cpp" "engine/graphics/backend/specialized/VertexAttributes.h" "engine/graphics/backend/specialized/VertexBuffers.h" "engine/graphics/backend/specialized/ElementBuffers.h" "engine/graphics/backend/specialized/PersistentBuffers.h" "engine/graphics/backend/specialized/LazyBuffers.h" "engine/graphics/backend/specialized/LightweightBuffers.h" "engine/graphics/backend/specialized/Mutability.h" "engine/core/math/Triangulation.h" "engine/core/math/Coordinates.h" "engine/core/math/ColoredGeometry.h" "engine/core/math/ColoredGeometry.cpp" "engine/core/math/Shapes.h" "engine/core/math/Coordinates.cpp" "engine/core/math/Shapes.cpp" "engine/graphics/extensions/SpriteAtlas.h" "engine/graphics/extensions/SpriteAtlas.cpp" "engine/core/platform/WindowResize.h" "engine/core/platform/WindowResize.cpp" "engine/registries/graphics/TextureRegistry.h" "engine/registries/graphics/TextureRegistry.cpp" "engine/registries/graphics/extensions/SpriteAtlases.h" "engine/registries/graphics/extensions/SpriteAtlases.cpp" "gen/archetypes/PolygonCrop.h" "gen/archetypes/PolygonCrop.cpp" "gen/archetypes/SpriteMatch.h" "gen/archetypes/SpriteMatch.cpp" "gen/archetypes/EllipsePair.h" "gen/archetypes/EllipsePair.cpp" "gen/archetypes/Jumble.h" "gen/archetypes/Jumble.cpp" "gen/archetypes/BKG.h" "gen/archetypes/BKG.cpp" "src/PlayerController.h" "src/PlayerController.cpp")

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
