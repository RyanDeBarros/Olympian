#include "ParticleSystem.h"

#include "graphics/particles/ShaderStructs.h"
#include "graphics/resources/Shaders.h"
#include "graphics/backend/basic/Shader.h"
#include "core/util/Time.h"

namespace oly::rendering
{
	void internal::ParticleSystemManager::on_tick()
	{
		for (ParticleSystem* system : systems)
			if (system->auto_tick) [[likely]]
				system->on_tick();
	}

	ParticleSystem::BufferList::ParticleDoubleBuffer::ParticleDoubleBuffer(GLuint max_particles)
		: a(max_particles * sizeof(particles::internal::Particle), 0), b(max_particles * sizeof(particles::internal::Particle), 0)
	{
	}

	void ParticleSystem::BufferList::ParticleDoubleBuffer::swap() const
	{
		state = !state;
	}

	const graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& ParticleSystem::BufferList::ParticleDoubleBuffer::in() const
	{
		return state ? a : b;
	}

	graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& ParticleSystem::BufferList::ParticleDoubleBuffer::in()
	{
		return state ? a : b;
	}

	const graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& ParticleSystem::BufferList::ParticleDoubleBuffer::out() const
	{
		return state ? b : a;
	}

	graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& ParticleSystem::BufferList::ParticleDoubleBuffer::out()
	{
		return state ? b : a;
	}

	ParticleSystem::BufferList::BufferList(GLuint particle_capacity)
		: particles(particle_capacity), emitter(sizeof(particles::internal::EmitterParams)),
		draw_command(sizeof(DrawArraysIndirectCommand)), ps_data(sizeof(particles::internal::ParticleSystemData))
	{
		draw_command.send(0, DrawArraysIndirectCommand{
			.count = 4,
			.primCount = 0,
			.first = 0,
			.baseVertex = 0,
			});

		ps_data.send(0, particles::internal::ParticleSystemData{
			.max_time_elapsed_bits = 0
			});
	}

	ParticleSystem::ParticleSystem(particles::ParticleEmitter&& emitter, GLuint particle_capacity, GLushort compute_threads)
		: buffers(particle_capacity), particle_capacity(particle_capacity), compute_threads(compute_threads)
	{
		internal::ParticleSystemManager::instance().systems.insert(this);
		emitters.push_back(std::move(emitter));
		init();
	}

	ParticleSystem::ParticleSystem(std::vector<particles::ParticleEmitter>&& emitters, GLuint particle_capacity, GLushort compute_threads)
		: buffers(particle_capacity), particle_capacity(particle_capacity), compute_threads(compute_threads), emitters(std::move(emitters))
	{
		internal::ParticleSystemManager::instance().systems.insert(this);
		init();
	}

	ParticleSystem::ParticleSystem(size_t emitter_count, GLuint particle_capacity, GLushort compute_threads)
		: buffers(particle_capacity), particle_capacity(particle_capacity), compute_threads(compute_threads)
	{
		internal::ParticleSystemManager::instance().systems.insert(this);
		init();
		for (size_t _ = 0; _ < emitter_count; ++_)
			emitters.push_back({});
	}

	ParticleSystem::~ParticleSystem()
	{
		internal::ParticleSystemManager::instance().systems.erase(this);
	}

	void ParticleSystem::init()
	{
		shaders = {
			.compute_spawn_ref = graphics::internal_shaders::particle_compute_spawn(compute_threads),
			.compute_update_ref = graphics::internal_shaders::particle_compute_update(compute_threads),
			.renderer = graphics::internal_shaders::particle_renderer
		};

		shaders.compute_spawn = *shaders.compute_spawn_ref;
		shaders.compute_update = *shaders.compute_update_ref;

		shader_locations.compute_spawn = {
			.time = glGetUniformLocation(shaders.compute_spawn, "uTime"),
			.spawn_count = glGetUniformLocation(shaders.compute_spawn, "uSpawnCount"),
			.transform = glGetUniformLocation(shaders.compute_spawn, "uTransform")
		};

		shader_locations.compute_update = {
			.delta_time = glGetUniformLocation(shaders.compute_update, "uDeltaTime"),
			.in_prim_count = glGetUniformLocation(shaders.compute_update, "uInPrimCount")
		};

		shader_locations.renderer = {
			.projection = glGetUniformLocation(shaders.renderer, "uProjection"),
			.transform = glGetUniformLocation(shaders.renderer, "uTransform"),
			.reverse_draw_order = glGetUniformLocation(shaders.renderer, "uReverseDrawOrder")
		};
	}

	void ParticleSystem::on_tick()
	{
		for (particles::ParticleEmitter& emitter : emitters)
			emitter.on_tick(time_elapsed - last_tick_time);
		last_tick_time = time_elapsed;

		time_elapsed += TIME.delta();
	}

	void ParticleSystem::render() const
	{
		for (const particles::ParticleEmitter& emitter : emitters)
			spawn_particles(emitter);

		GLuint in_primitive_count = buffers.draw_command.receive(0, &DrawArraysIndirectCommand::primCount);
		if (in_primitive_count > 0)
		{
			buffers.draw_command.send(0, &DrawArraysIndirectCommand::primCount, GLuint(0));
			update_particles(in_primitive_count, time_elapsed - last_render_time);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			draw_particles();
			glDisable(GL_DEPTH_TEST);
			buffers.particles.swap();
		}

		last_render_time = time_elapsed;
	}

	void ParticleSystem::spawn_particles(const particles::ParticleEmitter& emitter) const
	{
		GLuint to_spawn = emitter.spawn_debt();
		if (to_spawn == 0)
			return;

		particles::internal::EmitterParams params;
		emitter.apply(params);
		buffers.emitter.send(0, params);
		glUseProgram(shaders.compute_spawn);
		glUniform1f(shader_locations.compute_spawn.time, time_elapsed);
		glUniform1ui(shader_locations.compute_spawn.spawn_count, to_spawn);
		glUniformMatrix3fv(shader_locations.compute_spawn.transform, 1, GL_FALSE, glm::value_ptr(transformer.global()));
		buffers.particles.in().bind_base(0);
		buffers.emitter.bind_base(1);
		buffers.draw_command.bind_base(2);
		graphics::dispatch_compute(to_spawn, 1, 1, compute_threads, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void ParticleSystem::update_particles(GLuint in_primitive_count, float delta_time) const
	{
		buffers.ps_data.send(0, &particles::internal::ParticleSystemData::max_time_elapsed_bits, GLuint(0));
		glUseProgram(shaders.compute_update);
		glUniform1f(shader_locations.compute_update.delta_time, delta_time);
		glUniform1ui(shader_locations.compute_update.in_prim_count, in_primitive_count);
		buffers.particles.in().bind_base(0);
		buffers.particles.out().bind_base(1);
		buffers.draw_command.bind_base(2);
		buffers.ps_data.bind_base(3);
		graphics::dispatch_compute(in_primitive_count, 1, 1, compute_threads, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void ParticleSystem::draw_particles() const
	{
		glBindVertexArray(vao);
		glUseProgram(shaders.renderer);
		glUniformMatrix3fv(shader_locations.renderer.projection, 1, GL_FALSE, glm::value_ptr(camera_invariant ? camera->invariant_projection_matrix() : camera->projection_matrix()));
		glUniformMatrix3fv(shader_locations.renderer.transform, 1, GL_FALSE, glm::value_ptr(transformer.global()));
		glUniform1ui(shader_locations.renderer.reverse_draw_order, (GLuint)age_sort);
		buffers.particles.out().bind_base(0);
		buffers.draw_command.bind_base(1);
		buffers.ps_data.bind_base(2);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffers.draw_command.buffer());
		glDrawArraysIndirect(GL_TRIANGLE_STRIP, (void*)0);
		// TODO v6 use SDF textures for shapes (ellipses, polygons, etc.). Could also add SDF functionality to sprites.
	}

	void ParticleSystem::add_emitter(particles::ParticleEmitter&& emitter)
	{
		emitters.push_back(std::move(emitter));
	}

	void ParticleSystem::remove_emitter(size_t i)
	{
		emitters.erase(emitters.begin() + i);
	}

	void ParticleSystem::set_particle_capacity(GLuint capacity)
	{
		buffers.particles.in().force_resize(capacity * sizeof(particles::internal::Particle));
		buffers.particles.out().force_resize(capacity * sizeof(particles::internal::Particle));
		particle_capacity = capacity;
	}
}
