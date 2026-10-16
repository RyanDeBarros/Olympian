#include "ParticleSystem.h"

#include "graphics/particles/ShaderStructs.h"
#include "graphics/resources/Shaders.h"
#include "graphics/backend/basic/Shader.h"
#include "core/util/Time.h"

namespace oly::rendering
{
    ParticleSystem::BufferList::BufferList(GLuint particle_capacity)
        : particles(
            { particle_capacity * static_cast<GLsizeiptr>(sizeof(particles::internal::Particle)), 0 },
            { particle_capacity * static_cast<GLsizeiptr>(sizeof(particles::internal::Particle)), 0 })
        , emitter(sizeof(particles::internal::EmitterParams))
        , draw_command(sizeof(DrawArraysIndirectCommand))
        , ps_data(sizeof(particles::internal::ParticleSystemData))
    {
        draw_command.send(0, DrawArraysIndirectCommand{
            .count = 4,
            .prim_count = 0,
            .first = 0,
            .base_vertex = 0 });

        ps_data.send(0, particles::internal::ParticleSystemData{
            .max_time_elapsed_bits = 0 });
    }

    ParticleSystem::ParticleSystem(
        particles::ParticleEmitter&& emitter,
        GLuint particle_capacity,
        GLushort compute_threads)
        : ITickService(TickPhase::Logic, TerminatePhase::Logic)
        , buffers_(particle_capacity)
        , particle_capacity_(particle_capacity)
        , compute_threads_(compute_threads)
    {
        emitters_.push_back(std::move(emitter));
        init();
    }

    ParticleSystem::ParticleSystem(
        std::vector<particles::ParticleEmitter>&& emitters,
        GLuint particle_capacity,
        GLushort compute_threads)
        : ITickService(TickPhase::Logic, TerminatePhase::Logic)
        , buffers_(particle_capacity)
        , particle_capacity_(particle_capacity)
        , compute_threads_(compute_threads)
        , emitters_(std::move(emitters))
    {
        init();
    }

    ParticleSystem::ParticleSystem(
        size_t emitter_count,
        GLuint particle_capacity,
        GLushort compute_threads)
        : ITickService(TickPhase::Logic, TerminatePhase::Logic)
        , buffers_(particle_capacity)
        , particle_capacity_(particle_capacity)
        , compute_threads_(compute_threads)
    {
        for (size_t _ = 0; _ < emitter_count; ++_)
            emitters_.push_back({});

        init();
    }

    void ParticleSystem::on_tick()
    {
        for (particles::ParticleEmitter& emitter : emitters_)
            emitter.on_tick(time_elapsed_ - last_tick_time_);
        
        last_tick_time_ = time_elapsed_;
        time_elapsed_ += TIME.delta();
    }

    void ParticleSystem::render() const
    {
        for (const particles::ParticleEmitter& emitter : emitters_)
            spawn_particles(emitter);

        GLuint in_primitive_count = buffers_.draw_command.receive(0, &DrawArraysIndirectCommand::prim_count);
        if (in_primitive_count > 0)
        {
            buffers_.draw_command.send(0, &DrawArraysIndirectCommand::prim_count, GLuint(0));
            update_particles(in_primitive_count, time_elapsed_ - last_render_time_);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);
            draw_particles();
            glDisable(GL_DEPTH_TEST);
            buffers_.particles.swap();
        }

        last_render_time_ = time_elapsed_;
    }

    float ParticleSystem::get_time_elapsed() const
    {
        return time_elapsed_;
    }

    const particles::ParticleEmitter& ParticleSystem::emitter(size_t i) const
    {
        return emitters_[i];
    }

    particles::ParticleEmitter& ParticleSystem::emitter(size_t i)
    {
        return emitters_[i];
    }

    void ParticleSystem::add_emitter(particles::ParticleEmitter&& emitter)
    {
        emitters_.push_back(std::move(emitter));
    }

    void ParticleSystem::remove_emitter(size_t i)
    {
        emitters_.erase(emitters_.begin() + i);
    }

    GLuint ParticleSystem::get_particle_capacity() const
    {
        return particle_capacity_;
    }

    void ParticleSystem::set_particle_capacity(GLuint capacity)
    {
        buffers_.particles.front().force_resize(capacity * sizeof(particles::internal::Particle));
        buffers_.particles.back().force_resize(capacity * sizeof(particles::internal::Particle));
        particle_capacity_ = capacity;
    }

    void ParticleSystem::init()
    {
        shaders_ = {
            .compute_spawn_ref = graphics::internal_shaders::particle_compute_spawn(compute_threads_),
            .compute_update_ref = graphics::internal_shaders::particle_compute_update(compute_threads_),
            .renderer = graphics::internal_shaders::particle_renderer()
        };

        shaders_.compute_spawn = *shaders_.compute_spawn_ref;
        shaders_.compute_update = *shaders_.compute_update_ref;

        shader_locations_.compute_spawn = {
            .time = glGetUniformLocation(shaders_.compute_spawn, "uTime"),
            .spawn_count = glGetUniformLocation(shaders_.compute_spawn, "uSpawnCount"),
            .transform = glGetUniformLocation(shaders_.compute_spawn, "uTransform")
        };

        shader_locations_.compute_update = {
            .delta_time = glGetUniformLocation(shaders_.compute_update, "uDeltaTime"),
            .in_prim_count = glGetUniformLocation(shaders_.compute_update, "uInPrimCount")
        };

        shader_locations_.renderer = {
            .projection = glGetUniformLocation(shaders_.renderer, "uProjection"),
            .transform = glGetUniformLocation(shaders_.renderer, "uTransform"),
            .reverse_draw_order = glGetUniformLocation(shaders_.renderer, "uReverseDrawOrder")
        };
    }

    void ParticleSystem::spawn_particles(const particles::ParticleEmitter& emitter) const
    {
        GLuint to_spawn = emitter.consume_spawn_debt();
        if (to_spawn == 0)
            return;

        particles::internal::EmitterParams params;
        emitter.apply(params);
        buffers_.emitter.send(0, params);
        glUseProgram(shaders_.compute_spawn);
        glUniform1f(shader_locations_.compute_spawn.time, time_elapsed_);
        glUniform1ui(shader_locations_.compute_spawn.spawn_count, to_spawn);
        glUniformMatrix3fv(shader_locations_.compute_spawn.transform, 1, GL_FALSE, glm::value_ptr(transformer.global()));
        buffers_.particles.front().bind_base(0);
        buffers_.emitter.bind_base(1);
        buffers_.draw_command.bind_base(2);
        graphics::dispatch_compute(to_spawn, 1, 1, compute_threads_, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void ParticleSystem::update_particles(GLuint in_primitive_count, float delta_time) const
    {
        buffers_.ps_data.send(0, &particles::internal::ParticleSystemData::max_time_elapsed_bits, GLuint(0));
        glUseProgram(shaders_.compute_update);
        glUniform1f(shader_locations_.compute_update.delta_time, delta_time);
        glUniform1ui(shader_locations_.compute_update.in_prim_count, in_primitive_count);
        buffers_.particles.front().bind_base(0);
        buffers_.particles.back().bind_base(1);
        buffers_.draw_command.bind_base(2);
        buffers_.ps_data.bind_base(3);
        graphics::dispatch_compute(in_primitive_count, 1, 1, compute_threads_, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void ParticleSystem::draw_particles() const
    {
        glBindVertexArray(vao_);
        glUseProgram(shaders_.renderer);

        glUniformMatrix3fv(
            shader_locations_.renderer.projection,
            1,
            GL_FALSE,
            glm::value_ptr(camera_invariant ? camera->invariant_projection_matrix() : camera->projection_matrix()));
        
        glUniformMatrix3fv(
            shader_locations_.renderer.transform,
            1,
            GL_FALSE,
            glm::value_ptr(transformer.global()));
        
        glUniform1ui(shader_locations_.renderer.reverse_draw_order, (GLuint)age_sort);
        buffers_.particles.back().bind_base(0);
        buffers_.draw_command.bind_base(1);
        buffers_.ps_data.bind_base(2);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffers_.draw_command.buffer());
        glDrawArraysIndirect(GL_TRIANGLE_STRIP, (void*)0);
    }
}
