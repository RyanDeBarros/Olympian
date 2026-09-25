#pragma once

#include "assets/KeyDecl.h"

#include <imtk.hpp>

#include <filesystem>

namespace oly::editor
{
	enum class IconResource : int;

	struct PathInfo
	{
		static bool IsImportFile(const std::filesystem::path& path);
		static imtk::texture GetIcon(const std::filesystem::path& path);
		static IconResource GetAssetIcon(detail::Key meta_type);
		static void RevealInExplorer(const std::filesystem::path& path, bool open_folder_contents);
		static std::string NameOf(const std::filesystem::path& path);
	};
}
