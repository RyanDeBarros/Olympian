#include "Shaders.h"

#include "graphics/backend/basic/Shader.h"
#include "core/util/IO.h"

namespace oly::graphics::internal_shaders
{
#ifndef OLYMPIAN_ENGINE_ABS_PATH
#error "OLYMPIAN_ENGINE_ABS_PATH macro is not defined! Did you forget to configure CMake?"
#endif

	static std::string shaders_dir = OLYMPIAN_ENGINE_ABS_PATH + std::string("/internal/shaders/"); // TODO v8 embed shaders into built binary - or at least copy shaders to project folder under '.detail' subfolder or something

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

	GLuint sprite_batch(GLushort modulations, GLushort anims)
	{
		SpriteBatchTemplate tmpl{ .modulations = modulations, .anims = anims };
		auto it = _sprite_batch_map.find(tmpl);
		if (it != _sprite_batch_map.end())
			return *it->second;
		else
		{
			auto _sprite_batch = std::make_unique<Shader>(std::vector<ShaderBufferSource>{
				{
					.buffer = io::read_template_file(shaders_dir + "sprite_batch.vert", {
						{ "/*$MODULATIONS*/", std::to_string(tmpl.modulations) },
						{ "/*$ANIMS*/", std::to_string(tmpl.anims) }
					}),
					.type = ShaderType::VERTEX
				},
				{
					.buffer = io::read_file(shaders_dir + "sprite_batch.frag"),
					.type = ShaderType::FRAGMENT
				},
			});

			GLuint shader = *_sprite_batch;
			_sprite_batch_map.emplace(tmpl, std::move(_sprite_batch));
			return shader;
		}
	}

	static std::unique_ptr<Shader> _polygon_batch = nullptr;
	GLuint polygon_batch;

	static std::unique_ptr<Shader> _ellipse_batch = nullptr;
	GLuint ellipse_batch;

	static std::unique_ptr<Shader> _particle_renderer = nullptr;
	static std::unique_ptr<Shader> _particle_compute_spawn = nullptr;
	static std::unique_ptr<Shader> _particle_compute_update = nullptr;
	GLuint particle_renderer;
	GLuint particle_compute_spawn;
	GLuint particle_compute_update;

	void load()
	{
		_polygon_batch = std::make_unique<Shader>(std::vector<ShaderPathSource>{
			{ .path = shaders_dir + "polygon_batch.vert", .type = ShaderType::VERTEX },
			{ .path = shaders_dir + "polygon_batch.frag", .type = ShaderType::FRAGMENT }
		});
		polygon_batch = *_polygon_batch;

		_ellipse_batch = std::make_unique<Shader>(std::vector<ShaderPathSource>{
			{ .path = shaders_dir + "ellipse_batch.vert", .type = ShaderType::VERTEX },
			{ .path = shaders_dir + "ellipse_batch.frag", .type = ShaderType::FRAGMENT }
		});
		ellipse_batch = *_ellipse_batch;

		// TODO v6 uncomment once shaders are written. Later, use template files similar to sprite batch in order to dynamically use different particle structures, emitter attributes, etc.
		//_particle_renderer = std::make_unique<Shader>(std::vector<ShaderPathSource>{
		//	{ .path = shaders_dir + "particles/particle.vert", .type = ShaderType::VERTEX },
		//	{ .path = shaders_dir + "particles/particle.frag", .type = ShaderType::FRAGMENT }
		//});
		//particle_renderer = *_particle_renderer;

		//_particle_compute_spawn = std::make_unique<Shader>(std::vector<ShaderPathSource>{
		//	{ .path = shaders_dir + "particles/spawn.comp", .type = ShaderType::COMPUTE }
		//});
		//particle_compute_spawn = *_particle_compute_spawn;

		//_particle_compute_update = std::make_unique<Shader>(std::vector<ShaderPathSource>{
		//	{.path = shaders_dir + "particles/update.comp", .type = ShaderType::COMPUTE }
		//});
		//particle_compute_update = *_particle_compute_update;
	}

	void unload()
	{
		_sprite_batch_map.clear();

		_polygon_batch.reset();
		polygon_batch = 0;

		_ellipse_batch.reset();
		ellipse_batch = 0;

		_particle_renderer.reset();
		particle_renderer = 0;
		_particle_compute_spawn.reset();
		particle_compute_spawn = 0;
		_particle_compute_update.reset();
		particle_compute_update = 0;
	}
}
