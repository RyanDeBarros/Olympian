#pragma once

#include "gui/graphics/Texture.h"

#include <filesystem>

namespace oly::editor
{
	struct PathInfo
	{
		static bool IsImportFile(const std::filesystem::path& path);
		static Texture GetIcon(const std::filesystem::path& path);
		static void RevealInExplorer(const std::filesystem::path& path);
	};
}
