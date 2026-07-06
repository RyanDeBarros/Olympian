#include "Trashcan.h"

#include "core/editor/ProjectInfo.h"
#include "core/PathInfo.h"

namespace oly::editor::fio
{
	static std::filesystem::path TrashRoot()
	{
		return ProjectInfo::Instance().EditorRoot() / ".trash";
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
		return !ec;
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
