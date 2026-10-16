#pragma once

#include "graphics/particles/AttributeGenerator.h"

#include "external/GL.h"
#include "external/GLM.h"

namespace oly::particles
{
    namespace internal
    {
        struct EmitterParams;
    }

    struct ParticleEmitter
    {
    private:
        mutable float spawn_debt_ = 0.0f;
        mutable float time_elapsed_ = 0.0f;

    public:
        GLuint max_particles;
        GLuint attached;

        AttributeGenerator1D lifetime;
        AttributeGenerator2D position;
        AttributeGenerator1D rotation;
        AttributeGenerator2D size;
        AttributeGenerator2D velocity;
        AttributeGenerator4D color;

        imp::poly<IParticleSpawner> spawner;
        float spawn_period = 1.0f;

        enum class Loop
        {
            Unbounded,
            Loop,
            OneShot
        } loop = Loop::Loop;

    public:
        ParticleEmitter();

        void apply(internal::EmitterParams&) const;

        void on_tick(float delta_time);
        GLuint consume_spawn_debt() const;
        float time_elapsed() const;
    };
}
