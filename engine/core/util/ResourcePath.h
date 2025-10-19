#pragma once

#include <filesystem>
#include <fstream>

#include "core/types/Meta.h"

namespace oly
{
	namespace context::internal
	{
		extern void set_resource_root(const std::string& root);
	}

	class ResourcePath
	{
		std::filesystem::path absolute;

	public:
		ResourcePath() = default;

		ResourcePath(const std::string& path) { set(path); }
		ResourcePath(std::string&& path) { set(std::move(path)); }
		ResourcePath(const char* path) { set(path); }
		ResourcePath(const std::filesystem::path& path) { set(dupl(path)); }
		ResourcePath(std::filesystem::path&& path) { set(std::move(path)); }

		ResourcePath& operator=(const std::string& path) { set(path); return *this; }
		ResourcePath& operator=(std::string&& path) { set(std::move(path)); return *this; }
		ResourcePath& operator=(const char* path) { set(path); return *this; }
		ResourcePath& operator=(const std::filesystem::path& path) { set(dupl(path)); return *this; }
		ResourcePath& operator=(std::filesystem::path&& path) { set(std::move(path)); return *this; }

	private:
		void set(std::filesystem::path&& path);

	public:
		std::filesystem::path get_absolute() const;
		bool has_extension() const { return absolute.has_extension(); }
		std::string extension() const { return absolute.extension().generic_string(); }

		template<typename... Extensions>
		bool extension_matches(const Extensions&... extensions) const
		{
			return is_in(extension(), extensions...);
		}

		ResourcePath get_import_path() const;
		bool is_import_path() const;

		std::ifstream get_ifstream(std::ios_base::openmode mode = std::ios_base::in) const { return std::ifstream(absolute, mode); }
		std::ofstream get_ofstream(std::ios_base::openmode mode = std::ios_base::out) const { return std::ofstream(absolute, mode); }
		std::fstream get_fstream(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) const { return std::fstream(absolute, mode); }

		bool empty() const { return absolute.empty(); }
		size_t hash() const { return std::hash<std::filesystem::path>{}(absolute); }
		bool operator==(const ResourcePath&) const = default;
	};
}

template<>
struct std::hash<oly::ResourcePath>
{
	size_t operator()(const oly::ResourcePath& f) const { return f.hash(); }
};
