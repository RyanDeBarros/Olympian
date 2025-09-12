#include "Shaders.h"

#include "graphics/backend/basic/Shaders.h"

namespace oly::graphics::internal_shaders
{
#ifndef OLYMPIAN_ENGINE_ABS_PATH
#error "OLYMPIAN_ENGINE_ABS_PATH macro is not defined! Did you forget to configure CMake?"
#endif

	static std::string shaders_dir = OLYMPIAN_ENGINE_ABS_PATH + std::string("/internal/shaders/"); // TODO v5 embed shaders into built binary

	static std::unique_ptr<Shader> _sprite_batch = nullptr;
	GLuint sprite_batch;
	static std::unique_ptr<Shader> _polygon_batch = nullptr;
	GLuint polygon_batch;
	static std::unique_ptr<Shader> _ellipse_batch = nullptr;
	GLuint ellipse_batch;

	void load()
	{
		_sprite_batch = std::make_unique<Shader>(ShaderPathSource{ .vertex_path = shaders_dir + "sprite_batch.vert", .fragment_path = shaders_dir + "sprite_batch.frag" });
		sprite_batch = *_sprite_batch;
		_polygon_batch = std::make_unique<Shader>(ShaderPathSource{ .vertex_path = shaders_dir + "polygon_batch.vert", .fragment_path = shaders_dir + "polygon_batch.frag" });
		polygon_batch = *_polygon_batch;
		_ellipse_batch = std::make_unique<Shader>(ShaderPathSource{ .vertex_path = shaders_dir + "ellipse_batch.vert", .fragment_path = shaders_dir + "ellipse_batch.frag" });
		ellipse_batch = *_ellipse_batch;
	}

	void unload()
	{
		_sprite_batch.reset();
		sprite_batch = 0;
		_polygon_batch.reset();
		polygon_batch = 0;
		_ellipse_batch.reset();
		ellipse_batch = 0;
	}
}
