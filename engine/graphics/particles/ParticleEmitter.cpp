#include "ParticleEmitter.h"

#include "graphics/particles/ShaderStructs.h"
#include "graphics/particles/distributions/Samplers.h"
#include "graphics/particles/distributions/Domains.h"
#include "graphics/particles/distributions/Spawners.h"

namespace oly::particles
{
	ParticleEmitter::ParticleEmitter()
		: max_particles(2000),
		attached(false),
		lifetime({ .sampler = make_polymorphic<UniformSampler1D>(), .domain = make_polymorphic<ConstantDomain1D>(3.0f) }),
		position({ .sampler = make_polymorphic<UniformSampler2D>(), .domain = make_polymorphic<ConstantDomain2D>(glm::vec2{}) }),
		rotation({ .sampler = make_polymorphic<UniformSampler1D>(), .domain = make_polymorphic<ConstantDomain1D>(0.0f) }),
		size({ .sampler = make_polymorphic<UniformSampler2D>(), .domain = make_polymorphic<ConstantDomain2D>(glm::vec2{ 10.0f, 10.0f }) }),
		velocity({ .sampler = make_polymorphic<UniformSampler2D>(), .domain = make_polymorphic<ConstantDomain2D>(glm::vec2{ 10.0f, 0.0f }) }),
		color({ .sampler = make_polymorphic<UniformSampler4D>(), .domain = make_polymorphic<ConstantDomain4D>(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f }) }),
		spawner(make_polymorphic<ConstantParticleSpawner>(10.0f))
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
		float t = loop == Loop::LOOP && spawn_period > 1e-5f ? fmod(_time_elapsed, spawn_period) : _time_elapsed;
		_spawn_debt += spawner->spawn_debt(t, delta_time, spawn_period);

		lifetime.on_tick(*this);
		position.on_tick(*this);
		rotation.on_tick(*this);
		size.on_tick(*this);
		velocity.on_tick(*this);
		color.on_tick(*this);

		_time_elapsed += delta_time;
	}

	GLuint ParticleEmitter::spawn_debt() const
	{
		GLuint to_spawn = (GLuint)_spawn_debt;
		_spawn_debt -= to_spawn;
		return to_spawn;
	}
}
