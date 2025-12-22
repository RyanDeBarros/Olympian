#include "ParticleEmitter.h"

#include "graphics/particles/Samplers.h"
#include "graphics/particles/Domains.h"
#include "graphics/particles/ShaderStructs.h"

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
		color({ .sampler = make_polymorphic<UniformSampler4D>(), .domain = make_polymorphic<ConstantDomain4D>(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f }) })
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

	void ParticleEmitter::on_tick(float delta_time) const
	{
		_spawn_debt += 20.0f * delta_time; // TODO v6 use distribution for spawn rate + emitter loop/period parameters (loop should be enum of LOOP, UNBOUNDED, ONE_SHOT).
	}

	GLuint ParticleEmitter::spawn_debt() const
	{
		GLuint to_spawn = (GLuint)_spawn_debt;
		_spawn_debt -= to_spawn;
		return to_spawn;
	}
}
