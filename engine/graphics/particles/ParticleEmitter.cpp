#include "ParticleEmitter.h"

#include "graphics/resources/Shaders.h"
#include "graphics/backend/basic/Shader.h"
#include "core/util/Time.h"

namespace oly::rendering
{
	void ParticleEmitter::on_tick(float delta_time) const
	{
		//_spawn_debt += 20.0f * delta_time; // TODO v6 use distribution for spawn rate + emitter loop/period parameters (loop should be enum of LOOP, UNBOUNDED, ONE_SHOT).
		// TODO v6 works better without delta_time? But if emitter will use distribution, it needs to use delta time. So somehow spawn_debt() is not accounting for delta time or something.
		_spawn_debt += 0.2f;
	}

	GLuint ParticleEmitter::spawn_debt() const
	{
		GLuint to_spawn = (GLuint)_spawn_debt;
		_spawn_debt -= to_spawn;
		return to_spawn;
	}

	ParticleSystem::BufferList::ParticleDoubleBuffer::ParticleDoubleBuffer(GLuint max_particles)
		: a(max_particles * sizeof(Particle), 0), b(max_particles * sizeof(Particle), 0)
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
		: particles(particle_capacity), emitter(sizeof(EmitterParams)), draw_command(sizeof(DrawArraysIndirectCommand))
	{
		draw_command.send(0, DrawArraysIndirectCommand{
			.count = 4,
			.primCount = 0,
			.first = 0,
			.baseVertex = 0,
			});
	}

	ParticleSystem::ParticleSystem(const EmitterParams& ctor, GLuint particle_capacity, GLushort compute_threads)
		: buffers(particle_capacity), particle_capacity(particle_capacity), compute_threads(compute_threads)
	{
		init();
		add_emitter(ctor);
	}

	ParticleSystem::ParticleSystem(const std::vector<EmitterParams>& ctors, GLuint particle_capacity, GLushort compute_threads)
		: buffers(particle_capacity), particle_capacity(particle_capacity), compute_threads(compute_threads)
	{
		init();

		for (const EmitterParams& ctor : ctors)
			add_emitter(ctor);
	}

	ParticleSystem::ParticleSystem(size_t emitter_count, GLuint particle_capacity, GLushort compute_threads)
		: buffers(particle_capacity), particle_capacity(particle_capacity), compute_threads(compute_threads)
	{
		init();
		for (size_t _ = 0; _ < emitter_count; ++_)
			add_emitter();
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
			.transform = glGetUniformLocation(shaders.renderer, "uTransform")
		};
	}

	// TODO v6 on_tick() for engine classes (ParticleSystem, SpriteAtlas, etc.) should be called internally in frame(). Use a registry system for this, similar to RigidBodyManager. But be careful, since like here on_tick() of data members may be called manually. Define boolean for whether a class should auto-call its on_tick().
	void ParticleSystem::on_tick()
	{
		for (ParticleEmitter& emitter : emitters)
			emitter.on_tick(time_elapsed - last_tick_time);
		last_tick_time = time_elapsed;

		time_elapsed += TIME.delta();
	}

	void ParticleSystem::render() const
	{
		for (const ParticleEmitter& emitter : emitters)
			spawn_particles(emitter.params, emitter.spawn_debt());

		GLuint in_primitive_count = buffers.draw_command.receive(0, &DrawArraysIndirectCommand::primCount);
		if (in_primitive_count > 0)
		{
			buffers.draw_command.send(0, &DrawArraysIndirectCommand::primCount, GLuint(0));
			update_particles(in_primitive_count, time_elapsed - last_render_time);
			draw_particles();
			buffers.particles.swap();
		}

		last_render_time = time_elapsed;
	}

	// TODO v6 this mixes draw order. Add support to force particle draw order to match emitter order?
	void ParticleSystem::spawn_particles(const EmitterParams& emitter, GLuint to_spawn) const
	{
		if (to_spawn == 0)
			return;

		buffers.emitter.send(0, emitter);
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
		glUseProgram(shaders.compute_update);
		glUniform1f(shader_locations.compute_update.delta_time, delta_time);
		glUniform1ui(shader_locations.compute_update.in_prim_count, in_primitive_count);
		buffers.particles.in().bind_base(0);
		buffers.particles.out().bind_base(1);
		buffers.draw_command.bind_base(2);
		graphics::dispatch_compute(in_primitive_count, 1, 1, compute_threads, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void ParticleSystem::draw_particles() const
	{
		glBindVertexArray(vao);
		glUseProgram(shaders.renderer);
		glUniformMatrix3fv(shader_locations.renderer.projection, 1, GL_FALSE, glm::value_ptr(camera_invariant ? camera->invariant_projection_matrix() : camera->projection_matrix()));
		glUniformMatrix3fv(shader_locations.renderer.transform, 1, GL_FALSE, glm::value_ptr(transformer.global()));
		buffers.particles.out().bind_base(0);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffers.draw_command.buffer());
		glDrawArraysIndirect(GL_TRIANGLE_STRIP, (void*)0);
		// TODO v6 use SDF textures for shapes (ellipses, polygons, etc.). Could also add SDF functionality to sprites.
	}

	void ParticleSystem::set_particle_capacity(GLuint capacity)
	{
		buffers.particles.in().force_resize(capacity * sizeof(Particle));
		buffers.particles.out().force_resize(capacity * sizeof(Particle));
		particle_capacity = capacity;
	}
}
