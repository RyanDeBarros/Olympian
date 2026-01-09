#include "Shaders.h"

#include "core/util/IO.h"
#include "core/containers/SmartReferenceLookup.h"

namespace oly::graphics::internal_shaders
{
#ifndef OLYMPIAN_ENGINE_ABS_PATH
#error "OLYMPIAN_ENGINE_ABS_PATH macro is not defined! Did you forget to configure CMake?"
#endif

	// TODO v8 embed shaders into built binary - or at least copy shaders to project folder under '.detail' subfolder or something
	static std::string shaders_dir = OLYMPIAN_ENGINE_ABS_PATH + std::string("/internal/shaders/");

	// TODO v8 GL_NV_gpu_shader5 only supported on NVIDIA GPUs. Add support for other GPUs.
	// TODO v9 separate OpenGL/GLFW into independent module - so that different API backends can be implemented, like DirectX and Vulkan

	// --------------------------------------------------------------------------------------------------------------------------------

	struct SpriteBatchConstructor
	{
		GLushort modulations;
		GLushort anims;

		bool operator==(const SpriteBatchConstructor& other) const = default;

		Shader operator()() const
		{
			ShaderBufferSource vertex = {
				.buffer = io::read_template_file(shaders_dir + "sprite_batch.vert", { { "/*$MODULATIONS*/", std::to_string(modulations) }, { "/*$ANIMS*/", std::to_string(anims) } }),
				.type = ShaderType::VERTEX
			};
			ShaderBufferSource fragment = {
				.buffer = io::read_file(shaders_dir + "sprite_batch.frag"),
				.type = ShaderType::FRAGMENT
			};
			return Shader({ std::move(vertex), std::move(fragment) });
		}

		size_t hash() const
		{
			return std::hash<GLushort>{}(modulations) ^ (std::hash<GLushort>{}(anims) << 1);
		}
	};

	static SmartReferenceLookup<Shader, SpriteBatchConstructor> _sprite_batch;

	SmartReference<Shader> sprite_batch(GLushort modulations, GLushort anims)
	{
		return _sprite_batch.get({ .modulations = modulations, .anims = anims });
	}

	// --------------------------------------------------------------------------------------------------------------------------------

	static std::unique_ptr<Shader> _polygon_batch = nullptr;
	GLuint polygon_batch;

	static std::unique_ptr<Shader> _ellipse_batch = nullptr;
	GLuint ellipse_batch;

	// --------------------------------------------------------------------------------------------------------------------------------

	static std::unique_ptr<Shader> _particle_renderer = nullptr;
	GLuint particle_renderer;

	// --------------------------------------------------------------------------------------------------------------------------------

	struct ParticleComputeSpawnConstructor
	{
		GLushort x_threads;

		bool operator==(const ParticleComputeSpawnConstructor& other) const = default;

		Shader operator()() const
		{
			return Shader({ {
				.buffer = io::read_template_file(shaders_dir + "particles/spawn.comp", { { "/*$X_THREADS*/", std::to_string(x_threads) } }),
				.type = ShaderType::COMPUTE
			} });
		}

		size_t hash() const
		{
			return std::hash<GLushort>{}(x_threads);
		}
	};

	static SmartReferenceLookup<Shader, ParticleComputeSpawnConstructor> _particle_compute_spawn;

	SmartReference<Shader> particle_compute_spawn(GLushort x_threads)
	{
		return _particle_compute_spawn.get({ .x_threads = x_threads });
	}

	// --------------------------------------------------------------------------------------------------------------------------------

	struct ParticleComputeUpdateConstructor
	{
		GLushort x_threads;

		bool operator==(const ParticleComputeUpdateConstructor& other) const = default;

		Shader operator()() const
		{
			return Shader({ {
				.buffer = io::read_template_file(shaders_dir + "particles/update.comp", { { "/*$X_THREADS*/", std::to_string(x_threads) } }),
				.type = ShaderType::COMPUTE
			} });
		}

		size_t hash() const
		{
			return std::hash<GLushort>{}(x_threads);
		}
	};

	static SmartReferenceLookup<Shader, ParticleComputeUpdateConstructor> _particle_compute_update;

	SmartReference<Shader> particle_compute_update(GLushort x_threads)
	{
		return _particle_compute_update.get({ .x_threads = x_threads });
	}

	// --------------------------------------------------------------------------------------------------------------------------------

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

		_particle_renderer = std::make_unique<Shader>(std::vector<ShaderPathSource>{
			{ .path = shaders_dir + "particles/particle.vert", .type = ShaderType::VERTEX },
			{ .path = shaders_dir + "particles/particle.frag", .type = ShaderType::FRAGMENT }
		});
		particle_renderer = *_particle_renderer;
	}

	void unload()
	{
		_sprite_batch.clear();

		_polygon_batch.reset();
		polygon_batch = 0;

		_ellipse_batch.reset();
		ellipse_batch = 0;

		_particle_renderer.reset();
		particle_renderer = 0;
		_particle_compute_spawn.clear();
		_particle_compute_update.clear();
	}
}
