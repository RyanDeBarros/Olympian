#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace oly::detail
{
	// TODO v7 use keys
	struct MetaMap
	{
		std::unordered_map<std::string, std::string> map;

		std::optional<std::string> get_type() const;
		bool has_type(const std::string_view type) const;
	};

	struct MetaSplitter
	{
		static MetaMap meta(const std::filesystem::path& file);
	};
}
