#pragma once

#include "external/TOML.h"

#include "core/math/Random.h"

#include "graphics/old/Particles.h" // LATER remove particles?

namespace oly::reg
{
	extern random::bound::Function load_random_bound_function(const TOMLNode& node);
	extern random::bound::Function2D load_random_bound_function_2d(const TOMLNode& node);
	extern random::domain2d::Domain load_random_domain_2d(const TOMLNode& node);

	extern particles::EmitterParams load_particle_emitter_params(const TOMLNode& node);
	extern particles::Emitter load_particle_emitter(const TOMLNode& node, const glm::vec4& projection_bounds);
	extern particles::ParticleSystem load_particle_system(const TOMLNode& node, const glm::vec4& projection_bounds);
	extern particles::ParticleSystem load_particle_system(const char* file, const glm::vec4& projection_bounds);
	extern particles::ParticleSystem load_particle_system(const std::string& file, const glm::vec4& projection_bounds);
}
