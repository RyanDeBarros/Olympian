#include "Shaders.h"

#include "graphics/backend/basic/Shaders.h"
#include "core/util/IO.h"

namespace oly::graphics::internal_shaders
{
#ifndef OLYMPIAN_ENGINE_ABS_PATH
#error "OLYMPIAN_ENGINE_ABS_PATH macro is not defined! Did you forget to configure CMake?"
#endif

	static std::string shaders_dir = OLYMPIAN_ENGINE_ABS_PATH + std::string("/internal/shaders/"); // TODO v6 embed shaders into built binary

	struct SpriteBatchTemplate
	{
		GLushort modulations;
		GLushort anims;

		bool operator==(SpriteBatchTemplate other) const { return modulations == other.modulations && anims == other.anims; }
	};

	struct SpriteBatchTemplateHash
	{
		size_t operator()(SpriteBatchTemplate tmpl) const
		{
			return std::hash<GLushort>{}(tmpl.modulations) ^ (std::hash<GLushort>{}(tmpl.anims) << 1);
		}
	};

	static std::unordered_map<SpriteBatchTemplate, std::unique_ptr<Shader>, SpriteBatchTemplateHash> _sprite_batch_map;

	static std::unique_ptr<Shader> _polygon_batch = nullptr;
	GLuint polygon_batch;

	static std::unique_ptr<Shader> _ellipse_batch = nullptr;
	GLuint ellipse_batch;

	GLuint sprite_batch(GLushort modulations, GLushort anims)
	{
		SpriteBatchTemplate tmpl{ .modulations = modulations, .anims = anims };
		auto it = _sprite_batch_map.find(tmpl);
		if (it != _sprite_batch_map.end())
			return *it->second;
		else
		{
			auto _sprite_batch = std::make_unique<Shader>(ShaderBufferSource{
				.vertex_buffer = io::read_template_file(shaders_dir + "sprite_batch.vert", {
				{ "/*$MODULATIONS*/", std::to_string(tmpl.modulations) },
				{ "/*$ANIMS*/", std::to_string(tmpl.anims) }
				}),
				.fragment_buffer = io::read_file(shaders_dir + "sprite_batch.frag") });
			GLuint shader = *_sprite_batch;
			_sprite_batch_map.emplace(tmpl, std::move(_sprite_batch));
			return shader;
		}
	}

	void load()
	{
		_polygon_batch = std::make_unique<Shader>(ShaderPathSource{ .vertex_path = shaders_dir + "polygon_batch.vert", .fragment_path = shaders_dir + "polygon_batch.frag" });
		polygon_batch = *_polygon_batch;

		_ellipse_batch = std::make_unique<Shader>(ShaderPathSource{ .vertex_path = shaders_dir + "ellipse_batch.vert", .fragment_path = shaders_dir + "ellipse_batch.frag" });
		ellipse_batch = *_ellipse_batch;
	}

	void unload()
	{
		_sprite_batch_map.clear();

		_polygon_batch.reset();
		polygon_batch = 0;

		_ellipse_batch.reset();
		ellipse_batch = 0;
	}
}
