#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace oly::detail
{
	class ResourcePath;
	enum class Key : unsigned long long;

	struct MetaMap
	{
		std::unordered_map<Key, std::string> map;

		std::optional<Key> get_type() const;
		bool has_type(Key type) const;
		std::string get_version() const;
	};

	struct MetaSplitter
	{
		static MetaMap decode_meta(const ResourcePath& file);
		static std::string encode_meta(const MetaMap& meta);
	};
}
