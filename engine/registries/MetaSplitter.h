#pragma once

#include "external/TOML.h"
#include "core/util/ResourcePath.h"

#include <unordered_map>

namespace oly::reg
{
	struct MetaMap
	{
		std::unordered_map<std::string, std::string> map;

		std::optional<std::string> get_type() const;
		bool has_type(const std::string& type) const;
	};

	struct MetaSplitter
	{
		static MetaMap meta(const ResourcePath& file);
	};
}
