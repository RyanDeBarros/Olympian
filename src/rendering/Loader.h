#pragma once

#include <toml/toml.hpp>

#include "Particles.h"

namespace oly
{
	namespace assets
	{
		extern toml::v3::parse_result load_toml(const char* file);
		extern toml::v3::parse_result load_toml(const std::string& file);
		extern particles::EmitterParams load_emitter_params(const toml::v3::node_view<toml::v3::node>& node);
	}
}
