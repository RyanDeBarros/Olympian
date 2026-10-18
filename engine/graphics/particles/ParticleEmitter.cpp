#include "ParticleEmitter.h"

#include "graphics/particles/ShaderStructs.h"

#include "graphics/particles/implementations/Domains.h"
#include "graphics/particles/implementations/Samplers.h"
#include "graphics/particles/implementations/Spawners.h"

namespace oly::particles
{
    ParticleEmitter::ParticleEmitter()
        : max_particles(2000)
        , attached(false)
        , lifetime({
            .sampler = imp::make_poly<UniformSampler1D>(),
            .domain = imp::make_poly<ConstantDomain1D>(3.f) })
        , position({
            .sampler = imp::make_poly<UniformSampler2D>(),
            .domain = imp::make_poly<ConstantDomain2D>(glm::vec2{}) })
        , rotation({
            .sampler = imp::make_poly<UniformSampler1D>(),
            .domain = imp::make_poly<ConstantDomain1D>(0.f) })
        , size({
            .sampler = imp::make_poly<UniformSampler2D>(),
            .domain = imp::make_poly<ConstantDomain2D>(glm::vec2{ 10.f, 10.f }) })
        , velocity({
            .sampler = imp::make_poly<UniformSampler2D>(),
            .domain = imp::make_poly<ConstantDomain2D>(glm::vec2{ 10.f, 0.f }) })
        , color({
            .sampler = imp::make_poly<UniformSampler4D>(),
            .domain = imp::make_poly<ConstantDomain4D>(glm::vec4{ 1.f, 0.f, 0.f, 1.f }) })
        , spawner(imp::make_poly<ConstantParticleSpawner>(10.f))
    {
    }

    void ParticleEmitter::apply(internal::EmitterParams& params) const
    {
        params.max_particles = max_particles;
        params.attached = attached;
        lifetime.apply(params.lifetime);
        position.apply(params.position);
        rotation.apply(params.rotation);
        size.apply(params.size);
        velocity.apply(params.velocity);
        color.apply(params.color);
    }

    void ParticleEmitter::on_tick(float delta_time)
    {
        const float t = loop == Loop::Loop && spawn_period > 1e-5f ? fmod(time_elapsed_, spawn_period) : time_elapsed_;
        spawn_debt_ += spawner->spawn_debt(t, delta_time, spawn_period);

        lifetime.on_tick(*this);
        position.on_tick(*this);
        rotation.on_tick(*this);
        size.on_tick(*this);
        velocity.on_tick(*this);
        color.on_tick(*this);

        time_elapsed_ += delta_time;
    }

    GLuint ParticleEmitter::consume_spawn_debt() const
    {
        GLuint to_spawn = (GLuint)spawn_debt_;
        spawn_debt_ -= to_spawn;
        return to_spawn;
    }

    float ParticleEmitter::time_elapsed() const
    {
        return time_elapsed_;
    }
}
