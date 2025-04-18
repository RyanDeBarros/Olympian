#pragma once

#include <toml/toml.hpp>

#include "Particles.h"

namespace oly
{
	namespace assets
	{
		extern toml::v3::parse_result load_toml(const char* file);
		extern toml::v3::parse_result load_toml(const std::string& file);
		typedef toml::v3::node_view<toml::v3::node> AssetNode;

		extern Transform2D load_transform_2d(const AssetNode& node);
		extern random::bound::Function load_random_bound_function(const AssetNode& node);
		extern random::bound::Function2D load_random_bound_function_2d(const AssetNode& node);
		extern random::domain2d::Domain load_random_domain_2d(const AssetNode& node);

		extern particles::EmitterParams load_particle_emitter_params(const AssetNode& node);
		extern particles::Emitter load_particle_emitter(const AssetNode& node, const glm::vec4& projection_bounds);
		extern particles::ParticleSystem load_particle_system(const AssetNode& node, const glm::vec4& projection_bounds);
		extern particles::ParticleSystem load_particle_system(const char* file, const glm::vec4& projection_bounds);
		extern particles::ParticleSystem load_particle_system(const std::string& file, const glm::vec4& projection_bounds);
	}
}
