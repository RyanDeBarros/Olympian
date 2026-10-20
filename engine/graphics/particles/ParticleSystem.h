#pragma once

#include "core/base/Transforms.h"

#include "graphics/Camera.h"

#include "graphics/backend/basic/Shader.h"
#include "graphics/backend/basic/VertexArrays.h"

#include "graphics/backend/specialized/LightweightBuffers.h"

#include "graphics/particles/ParticleEmitter.h"

#include <imp/swapper.hpp>

#include <unordered_set>

namespace oly::rendering
{
    class ParticleSystem;

    class ParticleSystem
        : public ITickService
    {
        struct DrawArraysIndirectCommand
        {
            GLuint count;
            GLuint prim_count;
            GLuint first;
            GLuint base_vertex;
        };

        struct BufferList
        {
            imp::mut_swapper<graphics::LightweightSSBO<graphics::Mutability::Immutable>> particles;
            graphics::LightweightSSBO<graphics::Mutability::Immutable> emitter;
            graphics::LightweightSSBO<graphics::Mutability::Immutable> draw_command;
            graphics::LightweightSSBO<graphics::Mutability::Immutable> ps_data;

            BufferList(GLuint particle_capacity);
        };

    private:
        static inline const GLuint kDefaultParticleCapacity = 2000;
        static inline const GLushort kComputeThreads = 64;

        graphics::VertexArray vao_;

        BufferList buffers_;

        struct
        {
            SmartReference<graphics::Shader> compute_spawn_ref = kRefNull;
            GLuint compute_spawn = 0;

            SmartReference<graphics::Shader> compute_update_ref = kRefNull;
            GLuint compute_update = 0;

            GLuint renderer = 0;
        } shaders_;

        struct
        {
            struct
            {
                GLint time;
                GLint spawn_count;
                GLint transform;
            } compute_spawn;

            struct
            {
                GLint delta_time;
                GLint in_prim_count;
            } compute_update;

            struct
            {
                GLint projection;
                GLint transform;
                GLint reverse_draw_order;
            } renderer;
        } shader_locations_;

        std::vector<particles::ParticleEmitter> emitters_;
        GLuint particle_capacity_;
        GLushort compute_threads_;

        float time_elapsed_ = 0.0f;
        mutable float last_tick_time_ = 0.0f;
        mutable float last_render_time_ = 0.0f;

    public:
        bool camera_invariant = false;
        bool auto_tick = true;

        enum class AgeSort
        {
            YoungOnOld,
            OldOnYoung
        } age_sort = AgeSort::YoungOnOld;

        rendering::Camera2DRef camera = kRefDefault;
        Transformer2D transformer;

    public:
        ParticleSystem(
            particles::ParticleEmitter&& emitter = {},
            GLuint particle_capacity = kDefaultParticleCapacity,
            GLushort compute_threads = kComputeThreads);
        ParticleSystem(
            std::vector<particles::ParticleEmitter>&& emitters,
            GLuint particle_capacity = kDefaultParticleCapacity,
            GLushort compute_threads = kComputeThreads);
        ParticleSystem(
            size_t emitter_count,
            GLuint particle_capacity = kDefaultParticleCapacity,
            GLushort compute_threads = kComputeThreads);

    public:
        void on_tick();
        void render() const;

        float get_time_elapsed() const;

        const particles::ParticleEmitter& emitter(size_t i = 0) const;
        particles::ParticleEmitter& emitter(size_t i = 0);
        void add_emitter(particles::ParticleEmitter&& emitter = {});
        void remove_emitter(size_t i);

        GLuint get_particle_capacity() const;
        void set_particle_capacity(GLuint capacity);

    private:
        void init();
        void spawn_particles(const particles::ParticleEmitter& emitter) const;
        void update_particles(GLuint in_primitive_count, float delta_time) const;
        void draw_particles() const;
    };
}
