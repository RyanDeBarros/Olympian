#include "Trashcan.h"

#include "core/editor/ProjectInfo.h"
#include "core/PathInfo.h"

#include "desc/SimpleField.h"

#include "definitions/Keys.h"

namespace oly::editor::fio
{
	static std::filesystem::path TrashRoot()
	{
		return ProjectInfo::Instance().EditorRoot() / ".trash";
	}

	static std::filesystem::path TrashManifest()
	{
		return TrashRoot() / "_manifest.json";
	}

	static bool TrashFolder(const detail::ResourcePath& resource, std::filesystem::path& out)
	{
		std::filesystem::path relative;
		if (!resource.resource_relative_path(relative))
			return false;

		out = TrashRoot() / relative;
		return true;
	}

	static size_t LatestVersion(const std::filesystem::path& trash_folder)
	{
		size_t version = 0;
		for (const auto& entry : std::filesystem::directory_iterator(trash_folder))
		{
			std::string name = PathInfo::NameOf(entry.path());
			size_t v;
			auto result = std::from_chars(name.c_str(), name.c_str() + name.size(), v);
			if (result.ec == std::errc{} && version < v)
				version = v;
		}
		return version;
	}

	static void PruneTrashFolder(std::filesystem::path trash_folder)
	{
		const auto root = TrashRoot();
		std::error_code ec;

		while (!std::filesystem::equivalent(trash_folder, root, ec) && !ec)
		{
			if (!std::filesystem::is_directory(trash_folder, ec) || ec)
				break;

			if (!std::filesystem::is_empty(trash_folder, ec) || ec)
				break;

			std::filesystem::remove(trash_folder, ec);
			if (ec)
				break;

			trash_folder = trash_folder.parent_path();
		}
	}

	static int64_t DirectorySize(const std::filesystem::path& path)
	{
		int64_t size = 0;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
		{
			if (entry.is_regular_file())
				size += entry.file_size();
		}

		return size;
	}

	struct Manifest
	{
		struct Entry
		{
#define ENTRY_GENERATOR(M) \
		M(trash_path) \
		M(last_write_time) \
		M(size)

			SimpleField<std::string> trash_path;
			SimpleField<int64_t> last_write_time;
			SimpleField<int64_t> size;

			Entry() :
				trash_path("", detail::Key::Path),
				last_write_time(0, detail::Key::Time),
				size(0, detail::Key::Size)
			{
			}
			
			LOAD_DUMP_SIMPLE_FIELDS_IMPL(ENTRY_GENERATOR);

#undef ENTRY_GENERATOR
		};

		SimpleArrayDesc<Entry> entries;
		size_t total_size = 0;

		Manifest()
			: entries(detail::Key::Entry)
		{
		}

		void Load(const TOMLNode& node)
		{
			entries.Load(node);
			total_size = 0;
			for (const Entry& entry : entries)
				total_size += *entry.size;
		}

		void Dump(toml::table& table)
		{
			entries.Dump(table);
		}

		void Add(const std::filesystem::path& local_trash_path)
		{
			const size_t size = DirectorySize(local_trash_path);
			Entry entry;
			*entry.trash_path = local_trash_path.generic_string();
			*entry.size = size;
			entries.descs.push_back(std::move(entry));
			total_size += size;
		}

		void Remove(const std::filesystem::path& local_trash_path)
		{
			for (auto it = entries.begin(); it != entries.end(); ++it)
			{
				if (*it->trash_path == local_trash_path.generic_string())
				{
					total_size -= *it->size;
					entries.descs.erase(it);
					break;
				}
			}
		}

		void Prune(size_t size_limit)
		{
			for (auto& entry : entries)
			{
				*entry.last_write_time = std::filesystem::last_write_time(TrashRoot() / *entry.trash_path)
						.time_since_epoch().count();
			}

			// TODO v9.2 sort by last_write_time and enforce size_limit
		}
	};

	struct ManifestIO
	{
		Manifest manifest;

		ManifestIO()
		{
			auto path = TrashManifest();
			if (std::filesystem::exists(path))
				manifest.Load((TOMLNode)toml::parse_file(path.string()));
		}

		ManifestIO(const ManifestIO&) = delete;
		ManifestIO(ManifestIO&&) = delete;

		~ManifestIO()
		{
			toml::table table;
			manifest.Dump(table);
			std::ofstream os(TrashManifest());
			os << table;
		}

		const Manifest& operator*() const
		{
			return manifest;
		}

		Manifest& operator*()
		{
			return manifest;
		}

		const Manifest* operator->() const
		{
			return &manifest;
		}

		Manifest* operator->()
		{
			return &manifest;
		}
	};

	bool Trashcan::Delete(const detail::ResourcePath& resource)
	{
		std::filesystem::path trash_folder;
		if (!TrashFolder(resource, trash_folder))
			return false;

		std::error_code ec;
		std::filesystem::create_directories(trash_folder, ec);
		if (ec)
			return false;

		const size_t version = LatestVersion(trash_folder) + 1;
		std::filesystem::path version_path = trash_folder / std::to_string(version);

		std::filesystem::rename(resource.get_absolute(), version_path, ec);
		if (ec)
			return false;

		// TODO v9.2 enfore trash size limit: by default 5GB/10GB (can range from 1GB (low) to 20GB (high)) in preferences

		return true;
	}

	bool Trashcan::Restore(const detail::ResourcePath& resource)
	{
		std::filesystem::path trash_folder;
		if (!TrashFolder(resource, trash_folder))
			return false;

		if (!std::filesystem::is_directory(trash_folder))
			return false;

		const size_t version = LatestVersion(trash_folder);
		std::filesystem::path version_path = trash_folder / std::to_string(version);

		if (!std::filesystem::exists(version_path))
			return false;

		resource.create_parents();
		if ((std::filesystem::is_regular_file(version_path) && resource.is_directory()) || (std::filesystem::is_directory(version_path) && resource.is_file()))
			return false;

		std::error_code ec;
		std::filesystem::rename(version_path, resource.get_absolute(), ec);
		if (ec)
			return false;

		PruneTrashFolder(trash_folder);

		return true;
	}
}
