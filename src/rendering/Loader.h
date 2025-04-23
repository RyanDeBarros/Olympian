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

		extern bool parse_int(const AssetNode& node, const std::string& name, int& v);
		extern bool parse_vec2(const toml::v3::array* arr, glm::vec2& v);
		inline bool parse_vec2(const AssetNode& node, const std::string& name, glm::vec2& v) { return parse_vec2(node[name].as_array(), v); }
		extern bool parse_vec4(const toml::v3::array* arr, glm::vec4& v);
		inline bool parse_vec4(const AssetNode& node, const std::string& name, glm::vec4& v) { return parse_vec4(node[name].as_array(), v); }
		extern Transform2D load_transform_2d(const AssetNode& node);
		inline Transform2D load_transform_2d(const AssetNode& node, const char* name) { return load_transform_2d(node[name]); }
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
