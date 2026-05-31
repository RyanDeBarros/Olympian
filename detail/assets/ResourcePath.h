#pragma once

#include <filesystem>
#include <fstream>

namespace oly::detail
{
	template <typename T, typename... Set>
	constexpr bool is_in(const T& v, const Set&... set)
	{
		return ((v == set) || ...);
	}

	class ResourcePath
	{
		static std::filesystem::path resource_root;

		std::filesystem::path absolute;

	public:
		ResourcePath();

		ResourcePath(const std::string_view path, const ResourcePath& relative_to = {}) { set(path, relative_to); }
		ResourcePath(const std::string& path, const ResourcePath& relative_to = {}) { set(path, relative_to); }
		ResourcePath(std::string&& path, const ResourcePath& relative_to = {}) { set(std::move(path), relative_to); }
		ResourcePath(const char* path, const ResourcePath& relative_to = {}) { set(path, relative_to); }
		ResourcePath(const std::filesystem::path& path, const ResourcePath& relative_to = {}) { set(std::filesystem::path(path), relative_to); }
		ResourcePath(std::filesystem::path&& path, const ResourcePath& relative_to = {}) { set(std::move(path), relative_to); }

		ResourcePath& operator=(const std::string_view path) { set(path, {}); return *this; }
		ResourcePath& operator=(const std::string& path) { set(path, {}); return *this; }
		ResourcePath& operator=(std::string&& path) { set(std::move(path), {}); return *this; }
		ResourcePath& operator=(const char* path) { set(path, {}); return *this; }
		ResourcePath& operator=(const std::filesystem::path& path) { set(std::filesystem::path(path), {}); return *this; }
		ResourcePath& operator=(std::filesystem::path&& path) { set(std::move(path), {}); return *this; }

		static void set_resource_root(const std::filesystem::path& root);

	private:
		void set(std::filesystem::path&& path, const ResourcePath& relative_to);

	public:
		std::filesystem::path get_absolute() const;
		bool has_extension() const { return absolute.has_extension(); }
		std::string extension() const { return absolute.extension().generic_string(); }
		void create_parents() const { std::filesystem::create_directories(absolute.parent_path()); }

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
struct std::hash<oly::detail::ResourcePath>
{
	size_t operator()(const oly::detail::ResourcePath& f) const { return f.hash(); }
};
