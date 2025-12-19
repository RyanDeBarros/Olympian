#include "ParticleEmitter.h"

#include "graphics/resources/Shaders.h"
#include "graphics/backend/basic/Shader.h"
#include "core/util/Time.h"

namespace oly::rendering
{
	ParticleEmitter::BufferList::ParticleDoubleBuffer::ParticleDoubleBuffer(GLuint max_particles)
		: a(max_particles * sizeof(Particle), 0), b(max_particles * sizeof(Particle), 0)
	{
	}

	void ParticleEmitter::BufferList::ParticleDoubleBuffer::swap() const
	{
		state = !state;
	}

	const graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& ParticleEmitter::BufferList::ParticleDoubleBuffer::in() const
	{
		return state ? a : b;
	}

	const graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& ParticleEmitter::BufferList::ParticleDoubleBuffer::out() const
	{
		return state ? b : a;
	}

	ParticleEmitter::BufferList::BufferList(const ParticleEmitterParams& params)
		: particles(params.max_particles), emitter(sizeof(ParticleEmitterParams)), draw_command(sizeof(DrawElementsIndirectCommand))
	{
		emitter.send(0, params);
		reset_draw_command();
	}

	GLuint ParticleEmitter::BufferList::primitive_count() const
	{
		return draw_command.receive(0, &DrawElementsIndirectCommand::primCount);
	}

	void ParticleEmitter::BufferList::reset_draw_command() const
	{
		draw_command.send(0, DrawElementsIndirectCommand{
			.count = 4,
			.primCount = 0,
			.first = 0,
			.baseVertex = 0,
			});
	}

	ParticleEmitter::ParticleEmitter(const ParticleEmitterParams& params)
		: buffers(params)
	{
		shaders = {
			.compute_spawn = graphics::internal_shaders::particle_compute_spawn,
			.compute_update = graphics::internal_shaders::particle_compute_update,
			.renderer = graphics::internal_shaders::particle_renderer
		};

		shader_locations.compute_spawn = {
			.time = glGetUniformLocation(shaders.compute_spawn, "uTime"),
			.spawn_count = glGetUniformLocation(shaders.compute_spawn, "uSpawnCount")
		};

		shader_locations.compute_update = {
			.delta_time = glGetUniformLocation(shaders.compute_update, "uDeltaTime"),
			.in_prim_count = glGetUniformLocation(shaders.compute_update, "uInPrimCount")
		};

		shader_locations.renderer = {
			.transform = glGetUniformLocation(shaders.renderer, "uTransform"),
		};
	}

	// TODO v6 on_tick() for engine classes (ParticleEmitter, SpriteAtlas, etc.) should be called internally in frame(). Use a registry system for this, similar to RigidBodyManager.
	void ParticleEmitter::on_tick() const
	{
		// TODO v6 remove
		ParticleEmitterParams params;
		params.velocity = (glm::vec2)UnitVector2D(time_elapsed) * 100.0f;
		buffers.emitter.send(0, params);

		spawn_particles(1); // TODO v6 compute to-spawn debt

		time_elapsed += TIME.delta(); // TODO v6 eventually, when on_tick() and draw()/render() are on different threads (although may not be possible here because of compute shader -> could accumulate to-spawn debt on-tick but then dispatch_compute() for spawn compute shader in render()), make sure to distinguish between delta time for tick thread and for draw thread.
	}
	
	void ParticleEmitter::render() const
	{
		GLuint in_primitive_count = buffers.primitive_count();
		if (in_primitive_count > 0)
		{
			buffers.reset_draw_command();
			update_particles(in_primitive_count, time_elapsed - last_render_time);
			draw_particles();
			buffers.particles.swap();
		}

		last_render_time = time_elapsed;
	}

	void ParticleEmitter::spawn_particles(GLuint to_spawn) const
	{
		if (to_spawn == 0)
			return;

		glUseProgram(shaders.compute_spawn);
		glUniform1f(shader_locations.compute_spawn.time, time_elapsed);
		glUniform1ui(shader_locations.compute_spawn.spawn_count, to_spawn);
		buffers.particles.in().bind_base(0);
		buffers.emitter.bind_base(1);
		buffers.draw_command.bind_base(2);
		graphics::dispatch_compute(to_spawn, 1, 1, 64, 1, 1); // TODO v6 the x_threads must match layout(local_size_x = ...) in; -> make sure to update this when changing to template.
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void ParticleEmitter::update_particles(GLuint in_primitive_count, float delta_time) const
	{
		glUseProgram(shaders.compute_update);
		glUniform1f(shader_locations.compute_update.delta_time, delta_time);
		glUniform1ui(shader_locations.compute_update.in_prim_count, in_primitive_count);
		buffers.particles.in().bind_base(0);
		buffers.particles.out().bind_base(1);
		buffers.draw_command.bind_base(2);
		graphics::dispatch_compute(in_primitive_count, 1, 1, 64, 1, 1); // TODO v6 the x_threads must match layout(local_size_x = ...) in; -> make sure to update this when changing to template.
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void ParticleEmitter::draw_particles() const
	{
		glBindVertexArray(vao);
		glUseProgram(shaders.renderer);
		// TODO v6 camera invariance
		// TODO v6 free/solid particles: if emitter is moving, do particles follow (relative motion), or do they continue on their trajectory (independent of emitter motion). Implemented by caching the initial emitter transform in particle data.
		glUniformMatrix3fv(shader_locations.renderer.transform, 1, GL_FALSE, glm::value_ptr(camera->projection_matrix() * transformer.global()));
		buffers.particles.out().bind_base(0);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffers.draw_command.buffer());
		glDrawArraysIndirect(GL_TRIANGLE_STRIP, (void*)0);
		// TODO v6 use SDF textures for shapes (ellipses, polygons, etc.). Could also add SDF functionality to sprites.
	}
}
