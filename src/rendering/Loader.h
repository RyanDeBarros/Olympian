#pragma once

#include <toml/toml.hpp>

#include "Particles.h"

namespace oly
{
	namespace assets
	{
		extern toml::v3::parse_result load_toml(const char* file);
		extern toml::v3::parse_result load_toml(const std::string& file);

		extern Transform2D load_transform_2d(const toml::v3::node_view<toml::v3::node>& node);
		extern random::bound::Function load_random_bound_function(const toml::v3::node_view<toml::v3::node>& node);
		extern random::bound::Function2D load_random_bound_function_2d(const toml::v3::node_view<toml::v3::node>& node);
		extern random::domain2d::Domain load_random_domain_2d(const toml::v3::node_view<toml::v3::node>& node);

		extern particles::EmitterParams load_emitter_params(const toml::v3::node_view<toml::v3::node>& node);
	}
}
