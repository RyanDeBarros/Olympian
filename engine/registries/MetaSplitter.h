#pragma once

#include "external/TOML.h"

#include <unordered_map>

namespace oly::reg
{
	struct MetaMap
	{
		std::unordered_map<std::string, std::string> map;

		bool has_type(const std::string& type) const;
	};

	struct MetaSplitter
	{
		static MetaMap meta(const std::string& filepath);
	};
}
