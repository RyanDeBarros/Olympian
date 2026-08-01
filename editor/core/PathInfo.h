#pragma once

#include "gui/graphics/Texture.h"

#include "assets/KeyDecl.h"

#include <filesystem>

namespace oly::editor
{
	enum class IconResource : int;

	struct PathInfo
	{
		static bool IsImportFile(const std::filesystem::path& path);
		static Texture GetIcon(const std::filesystem::path& path);
		static IconResource GetAssetIcon(detail::Key meta_type);
		static void RevealInExplorer(const std::filesystem::path& path);
		static std::string NameOf(const std::filesystem::path& path);
	};
}
